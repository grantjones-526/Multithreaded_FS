#include "FileSystem.h"

#include <algorithm>
#include <cstring>

FileSystem::FileSystem() {
    pthread_mutex_init(&dir_mutex, NULL);
    pthread_mutex_init(&swoft_mutex, NULL);
    pthread_mutex_init(&vcb_mutex, NULL);
    pthread_mutex_init(&storage_mutex, NULL);
}

bool FileSystem::create(string file_name, int file_size) {
    pthread_mutex_lock(&dir_mutex);
    if (this->dir.check_entry(file_name)) {
        pthread_mutex_unlock(&dir_mutex);
        return 0;
    }

    pthread_mutex_lock(&vcb_mutex);
    int free_block = this->vcb.get_contiguous_blocks(file_size);
    if (free_block == -1) {
        pthread_mutex_unlock(&vcb_mutex);
        pthread_mutex_unlock(&dir_mutex);
        return 0;
    }
    pthread_mutex_unlock(&vcb_mutex);

    FCB* entry = new FCB(free_block, file_size);
    this->dir.add_entry(file_name, entry);
    pthread_mutex_unlock(&dir_mutex);
    return 1;
}

ptOFT_Entry* FileSystem::open(string file_name) {
    pthread_mutex_lock(&dir_mutex);
    FCB* fcb = this->dir.get_entry(file_name);
    if (fcb == nullptr) {
        pthread_mutex_unlock(&dir_mutex);
        return nullptr;
    }

    pthread_mutex_lock(&swoft_mutex);
    swOFT_Entry* sw_entry;
    if (this->sw_table.check_entry(file_name)) {
        sw_entry = this->sw_table.get_entry(file_name);
        sw_entry->reference_count++;
    } else {
        sw_entry = this->sw_table.add_entry(file_name, fcb);
    }
    pthread_mutex_unlock(&swoft_mutex);
    pthread_mutex_unlock(&dir_mutex);

    ptOFT_Entry* pt_entry = new ptOFT_Entry(0, sw_entry);
    return pt_entry;
}

bool FileSystem::close(string file_name, ptOFT* pt_table) {
    ptOFT_Entry* pt_entry = pt_table->get_entry(file_name);
    if (pt_entry == nullptr) {
        return 0;
    }

    pthread_mutex_lock(&swoft_mutex);
    swOFT_Entry* sw_entry = pt_entry->swOFT_pointer;
    sw_entry->reference_count--;

    if (sw_entry->reference_count == 0) {
        this->sw_table.erase_entry(file_name);
    }
    pthread_mutex_unlock(&swoft_mutex);

    pt_table->erase_entry(file_name);
    return 1;
}

char* FileSystem::read(string file_name, ptOFT* pt_table) {
    ptOFT_Entry* pt_entry = pt_table->get_entry(file_name);
    if (pt_entry == nullptr) {
        return nullptr;
    }

    pthread_mutex_lock(&swoft_mutex);
    swOFT_Entry* sw_entry = pt_entry->swOFT_pointer;
    FCB* fcb = sw_entry->fcb_pointer;
    int start_block = fcb->get_start_block();
    int file_size = fcb->get_file_size();
    pthread_mutex_unlock(&swoft_mutex);

    pthread_mutex_lock(&storage_mutex);
    char* buffer = new char[file_size * 2048];
    for (int i = 0; i < file_size; i++) {
        this->storage.read_block(start_block + i, buffer + (i * 2048));
    }
    pthread_mutex_unlock(&storage_mutex);

    return buffer;
} 

bool FileSystem::write(string file_name, ptOFT* pt_table, char* data, int num_bytes) {
    ptOFT_Entry* pt_entry = pt_table->get_entry(file_name);
    if (pt_entry == nullptr) {
        return 0;
    }

    pthread_mutex_lock(&swoft_mutex);
    swOFT_Entry* sw_entry = pt_entry->swOFT_pointer;
    FCB* fcb = sw_entry->fcb_pointer;
    int start_block = fcb->get_start_block();
    int offset = sw_entry->file_offset;

    int target_block = start_block + (offset / 2048);
    int block_offset = offset % 2048;

    pthread_mutex_lock(&storage_mutex);
    int bytes_written = 0;
    while (bytes_written < num_bytes) {
        char block_buf[2048];
        this->storage.read_block(target_block, block_buf);

        int space_in_block = 2048 - block_offset;
        int to_write = min(space_in_block, num_bytes - bytes_written);
        memcpy(block_buf + block_offset, data + bytes_written, to_write);

        this->storage.write_block(target_block, block_buf);

        bytes_written += to_write;
        target_block++;
        block_offset = 0;
    }
    pthread_mutex_unlock(&storage_mutex);

    sw_entry->file_offset += num_bytes;
    pthread_mutex_unlock(&swoft_mutex);
    return 1;
}

bool FileSystem::rename(string old_name, string new_name) {
    pthread_mutex_lock(&dir_mutex);
    if (!this->dir.check_entry(old_name)) {
        pthread_mutex_unlock(&dir_mutex);
        return 0;
    }
    if (this->dir.check_entry(new_name)) {
        pthread_mutex_unlock(&dir_mutex);
        return 0;
    }

    FCB* fcb = this->dir.get_entry(old_name);
    this->dir.add_entry(new_name, fcb);
    this->dir.erase_entry(old_name);
    pthread_mutex_unlock(&dir_mutex);

    return 1;
}   

bool FileSystem::remove(string file_name) {
    pthread_mutex_lock(&dir_mutex);
    if (!this->dir.check_entry(file_name)) {
        pthread_mutex_unlock(&dir_mutex);
        return 0;
    }

    pthread_mutex_lock(&swoft_mutex);
    if (this->sw_table.check_entry(file_name)) {
        pthread_mutex_unlock(&swoft_mutex);
        pthread_mutex_unlock(&dir_mutex);
        return 0; // can't delete while file is open
    }
    pthread_mutex_unlock(&swoft_mutex);

    FCB* fcb = this->dir.get_entry(file_name);
    int start_block = fcb->get_start_block();
    int file_size = fcb->get_file_size();

    pthread_mutex_lock(&vcb_mutex);
    this->vcb.free_space(start_block, file_size);
    pthread_mutex_unlock(&vcb_mutex);

    this->dir.erase_entry(file_name);
    delete fcb;
    pthread_mutex_unlock(&dir_mutex);

    return 1;
}

FileSystem::~FileSystem() {
    pthread_mutex_destroy(&dir_mutex);
    pthread_mutex_destroy(&swoft_mutex);
    pthread_mutex_destroy(&vcb_mutex);
    pthread_mutex_destroy(&storage_mutex);
}