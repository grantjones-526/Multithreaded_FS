#include "FileSystem.h"

#include <algorithm>
#include <cstring>

// Set up all four mutex locks, one for each shared data structure.
FileSystem::FileSystem() {
    pthread_mutex_init(&directory_lock, NULL);
    pthread_mutex_init(&file_table_lock, NULL);
    pthread_mutex_init(&volume_lock, NULL);
    pthread_mutex_init(&disk_lock, NULL);
}

// Create a new file with the given name and size (in blocks).
// Finds contiguous free space on disk, creates a file control block,
// and adds the file to the directory.
// Returns false if the name already exists or there is not enough space.
bool FileSystem::create(string file_name, int file_size) {
    // Lock the directory so no other thread can create the same file name
    pthread_mutex_lock(&directory_lock);
    if (this->directory.check_entry(file_name)) {
        pthread_mutex_unlock(&directory_lock);
        return false; // file already exists
    }

    // Lock the volume control block to allocate disk space
    pthread_mutex_lock(&volume_lock);
    int start_block = this->volume.get_contiguous_blocks(file_size);
    if (start_block == -1) {
        pthread_mutex_unlock(&volume_lock);
        pthread_mutex_unlock(&directory_lock);
        return false; // not enough contiguous space
    }
    pthread_mutex_unlock(&volume_lock);

    // Create the file control block and add it to the directory
    FileControlBlock* new_fcb = new FileControlBlock(start_block, file_size);
    this->directory.add_entry(file_name, new_fcb);
    pthread_mutex_unlock(&directory_lock);
    return true;
}

// Open a file for reading, writing, or both.
// If another thread already has the file open, the open count goes up by 1.
// Returns a per-thread file entry, or nullptr if the file does not exist.
ThreadFileEntry* FileSystem::open(string file_name, string mode) {
    // Lock the directory to look up the file
    pthread_mutex_lock(&directory_lock);
    FileControlBlock* fcb = this->directory.get_entry(file_name);
    if (fcb == nullptr) {
        pthread_mutex_unlock(&directory_lock);
        return nullptr; // file not found
    }

    // Lock the system-wide open file table to update the open count
    pthread_mutex_lock(&file_table_lock);
    SystemFileEntry* system_entry;
    if (this->system_file_table.check_entry(file_name)) {
        // File is already open by another thread -- increase the count
        system_entry = this->system_file_table.get_entry(file_name);
        system_entry->open_count++;
    } else {
        // This is the first thread to open this file -- create a new entry
        system_entry = this->system_file_table.add_entry(file_name, fcb);
    }
    system_entry->access_mode = mode;
    pthread_mutex_unlock(&file_table_lock);
    pthread_mutex_unlock(&directory_lock);

    // Create a private entry for this thread (with its own file offset)
    ThreadFileEntry* thread_entry = new ThreadFileEntry(0, system_entry);
    return thread_entry;
}

// Close a file for the calling thread.
// Decreases the open count by 1. When the count reaches 0,
// the file is removed from the system-wide open file table.
// Returns false if the file is not in this thread's table.
bool FileSystem::close(string file_name, ThreadOpenFileTable* thread_table) {
    ThreadFileEntry* thread_entry = thread_table->get_entry(file_name);
    if (thread_entry == nullptr) {
        return false; // this thread doesn't have the file open
    }

    // Lock the system-wide table to safely update the open count
    pthread_mutex_lock(&file_table_lock);
    SystemFileEntry* system_entry = thread_entry->system_entry;
    system_entry->open_count--;

    if (system_entry->open_count == 0) {
        // No threads have this file open anymore -- remove it
        this->system_file_table.erase_entry(file_name);
    }
    pthread_mutex_unlock(&file_table_lock);

    // Remove from this thread's private table
    thread_table->erase_entry(file_name);
    return true;
}

// Read the entire file contents into a new buffer and return it.
// Returns nullptr if the file is not open or was not opened for reading.
// The caller must free the returned buffer when done.
char* FileSystem::read(string file_name, ThreadOpenFileTable* thread_table) {
    ThreadFileEntry* thread_entry = thread_table->get_entry(file_name);
    if (thread_entry == nullptr) {
        return nullptr; // this thread doesn't have the file open
    }

    // Lock the system-wide table to safely read the file's metadata
    pthread_mutex_lock(&file_table_lock);
    SystemFileEntry* system_entry = thread_entry->system_entry;

    // Check that the file was opened with read permission
    if (system_entry->access_mode.find('r') == string::npos) {
        pthread_mutex_unlock(&file_table_lock);
        return nullptr; // not opened for reading
    }

    FileControlBlock* fcb = system_entry->file_control_block;
    int start_block = fcb->get_start_block();
    int file_size = fcb->get_file_size();
    pthread_mutex_unlock(&file_table_lock);

    // Lock the disk and read each block of the file into a buffer
    pthread_mutex_lock(&disk_lock);
    char* buffer = new char[file_size * 2048];
    for (int i = 0; i < file_size; i++) {
        this->disk.read_block(start_block + i, buffer + (i * 2048));
    }
    pthread_mutex_unlock(&disk_lock);

    return buffer;
}

// Write data to a file starting at this thread's current position.
// Returns false if the file is not open, not opened for writing,
// or if the data would go past the end of the file's allocated space.
bool FileSystem::write(string file_name, ThreadOpenFileTable* thread_table, char* data, int num_bytes) {
    ThreadFileEntry* thread_entry = thread_table->get_entry(file_name);
    if (thread_entry == nullptr) {
        return false; // this thread doesn't have the file open
    }

    // Lock the system-wide table to read the file's metadata
    pthread_mutex_lock(&file_table_lock);
    SystemFileEntry* system_entry = thread_entry->system_entry;

    // Check that the file was opened with write permission
    if (system_entry->access_mode.find('w') == string::npos) {
        pthread_mutex_unlock(&file_table_lock);
        return false; // not opened for writing
    }

    FileControlBlock* fcb = system_entry->file_control_block;
    int start_block = fcb->get_start_block();
    int file_size = fcb->get_file_size();
    int offset = thread_entry->file_offset;
    int max_bytes = file_size * 2048;

    // Make sure we won't write past the file's allocated space
    if (offset + num_bytes > max_bytes) {
        pthread_mutex_unlock(&file_table_lock);
        return false; // would exceed file size
    }

    // Figure out which block and position within that block to start at
    int current_block = start_block + (offset / 2048);
    int position_in_block = offset % 2048;

    // Lock the disk and write the data block by block
    pthread_mutex_lock(&disk_lock);
    int bytes_written = 0;
    while (bytes_written < num_bytes) {
        // Read the existing block, update the relevant portion, write it back
        char block_buffer[2048];
        this->disk.read_block(current_block, block_buffer);

        int space_left_in_block = 2048 - position_in_block;
        int bytes_to_write = min(space_left_in_block, num_bytes - bytes_written);
        memcpy(block_buffer + position_in_block, data + bytes_written, bytes_to_write);

        this->disk.write_block(current_block, block_buffer);

        bytes_written += bytes_to_write;
        current_block++;
        position_in_block = 0; // after the first block, always start at the beginning
    }
    pthread_mutex_unlock(&disk_lock);
    pthread_mutex_unlock(&file_table_lock);

    // Move this thread's position forward (no lock needed -- it's private to this thread)
    thread_entry->file_offset += num_bytes;
    return true;
}

// Rename a file from old_name to new_name.
// Fails if old_name doesn't exist, new_name is already taken,
// or the file is currently open by any thread.
bool FileSystem::rename(string old_name, string new_name) {
    pthread_mutex_lock(&directory_lock);

    if (!this->directory.check_entry(old_name)) {
        pthread_mutex_unlock(&directory_lock);
        return false; // old name not found
    }
    if (this->directory.check_entry(new_name)) {
        pthread_mutex_unlock(&directory_lock);
        return false; // new name already taken
    }

    // Check that no thread has the file open
    pthread_mutex_lock(&file_table_lock);
    if (this->system_file_table.check_entry(old_name)) {
        pthread_mutex_unlock(&file_table_lock);
        pthread_mutex_unlock(&directory_lock);
        return false; // can't rename while file is open
    }
    pthread_mutex_unlock(&file_table_lock);

    // Move the directory entry to the new name (same file control block)
    FileControlBlock* fcb = this->directory.get_entry(old_name);
    this->directory.add_entry(new_name, fcb);
    this->directory.erase_entry(old_name);
    pthread_mutex_unlock(&directory_lock);

    return true;
}

// Delete a file and free its disk space.
// Fails if the file doesn't exist or is currently open by any thread.
bool FileSystem::remove(string file_name) {
    pthread_mutex_lock(&directory_lock);

    if (!this->directory.check_entry(file_name)) {
        pthread_mutex_unlock(&directory_lock);
        return false; // file not found
    }

    // Check that no thread has the file open
    pthread_mutex_lock(&file_table_lock);
    if (this->system_file_table.check_entry(file_name)) {
        pthread_mutex_unlock(&file_table_lock);
        pthread_mutex_unlock(&directory_lock);
        return false; // can't delete while file is open
    }
    pthread_mutex_unlock(&file_table_lock);

    // Get the file's location on disk so we can free those blocks
    FileControlBlock* fcb = this->directory.get_entry(file_name);
    int start_block = fcb->get_start_block();
    int file_size = fcb->get_file_size();

    // Free the blocks in the volume control block
    pthread_mutex_lock(&volume_lock);
    this->volume.free_space(start_block, file_size);
    pthread_mutex_unlock(&volume_lock);

    // Remove from directory and free the file control block
    this->directory.erase_entry(file_name);
    delete fcb;
    pthread_mutex_unlock(&directory_lock);

    return true;
}

// Destroy all mutex locks when the file system is cleaned up.
FileSystem::~FileSystem() {
    pthread_mutex_destroy(&directory_lock);
    pthread_mutex_destroy(&file_table_lock);
    pthread_mutex_destroy(&volume_lock);
    pthread_mutex_destroy(&disk_lock);
}
