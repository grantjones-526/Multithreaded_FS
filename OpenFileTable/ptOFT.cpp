#include "ptOFT.h"

// Create a per-thread entry with a handle, a link to the system-wide entry,
// and the file offset starting at the beginning of the file (0).
ThreadFileEntry::ThreadFileEntry(int handle, SystemFileEntry* system_entry) {
    this->handle = handle;
    this->system_entry = system_entry;
    this->file_offset = 0;
}

ThreadFileEntry::~ThreadFileEntry() {}

ThreadOpenFileTable::ThreadOpenFileTable() {}

// When a thread's table is destroyed, free all its entries.
ThreadOpenFileTable::~ThreadOpenFileTable() {
    for (auto& pair : this->open_files) {
        delete pair.second;
    }
}

// Add a file to this thread's table with the next available handle number.
ThreadFileEntry* ThreadOpenFileTable::add_entry(string file_name, SystemFileEntry* system_entry) {
    ThreadFileEntry* entry = new ThreadFileEntry(this->next_handle, system_entry);
    this->open_files[file_name] = entry;
    this->next_handle++;
    return entry;
}

// Look up a file in this thread's table. Returns nullptr if not found.
ThreadFileEntry* ThreadOpenFileTable::get_entry(string file_name) {
    if (!this->check_entry(file_name)) {
        return nullptr;
    }
    return this->open_files.at(file_name);
}

// Check whether this thread has a given file open.
bool ThreadOpenFileTable::check_entry(string file_name) {
    return this->open_files.count(file_name) > 0;
}

// Remove a file from this thread's table and free its entry.
void ThreadOpenFileTable::erase_entry(string file_name) {
    auto it = this->open_files.find(file_name);
    if (it != this->open_files.end()) {
        delete it->second;
        this->open_files.erase(it);
    }
}
