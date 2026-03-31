#include "Directory.h"

Directory::Directory() {}

// Add a new file to the directory.
void Directory::add_entry(string file_name, FileControlBlock* entry) {
    this->files.insert({file_name, entry});
}

// Look up a file by name. Returns its file control block, or nullptr if not found.
FileControlBlock* Directory::get_entry(string file_name) {
    if (!this->check_entry(file_name)) {
        return nullptr;
    }
    return this->files.at(file_name);
}

// Check whether a file exists in the directory.
bool Directory::check_entry(string file_name) {
    return this->files.count(file_name) > 0;
}

// Remove a file from the directory.
// this does not free the file control block itself.
void Directory::erase_entry(string file_name) {
    this->files.erase(file_name);
}

// When the directory is destroyed, free all file control blocks it owns.
Directory::~Directory() {
    for (auto& pair : this->files) {
        delete pair.second;
    }
}
