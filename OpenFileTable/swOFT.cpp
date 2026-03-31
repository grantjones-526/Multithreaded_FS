#include "swOFT.h"

using namespace std;

// When a file is first opened, its open count starts at 1.
SystemFileEntry::SystemFileEntry(FileControlBlock* fcb) {
    this->file_control_block = fcb;
    this->open_count = 1;
}

SystemOpenFileTable::SystemOpenFileTable() {}

SystemFileEntry::~SystemFileEntry() {}

// When the table is destroyed, free all remaining entries.
SystemOpenFileTable::~SystemOpenFileTable() {
    for (auto& pair : this->open_files) {
        delete pair.second;
    }
}

// Add a new file to the system-wide open file table.
SystemFileEntry* SystemOpenFileTable::add_entry(string file_name, FileControlBlock* fcb) {
    SystemFileEntry* entry = new SystemFileEntry(fcb);
    this->open_files[file_name] = entry;
    return entry;
}

// Look up a file by name. Returns nullptr if the file is not currently open.
SystemFileEntry* SystemOpenFileTable::get_entry(string file_name) {
    if (!this->check_entry(file_name)) {
        return nullptr;
    }
    return this->open_files.at(file_name);
}

// Check whether a file is currently in the system-wide open file table.
bool SystemOpenFileTable::check_entry(string file_name) {
    return this->open_files.count(file_name) > 0;
}

// Remove a file from the table and free its entry.
void SystemOpenFileTable::erase_entry(string file_name) {
    auto it = this->open_files.find(file_name);
    if (it != this->open_files.end()) {
        delete it->second;
        this->open_files.erase(it);
    }
}
