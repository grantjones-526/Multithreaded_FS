#pragma once

#include "FCB.h"
#include <map>
#include <string>

using namespace std;

// One entry in the system-wide open file table.
// Shared by all threads that have the same file open.
class SystemFileEntry {
public:
    SystemFileEntry(FileControlBlock* fcb);
    ~SystemFileEntry();

    int open_count;                    // how many threads have this file open
    string access_mode;                // "r", "w", or "rw"
    FileControlBlock* file_control_block;  // pointer to this file's metadata

private:
    string file_name;
};

// The system-wide open file table.
// Tracks every file that is currently open by any thread.
class SystemOpenFileTable {
public:
    SystemOpenFileTable();
    ~SystemOpenFileTable();
    SystemFileEntry* add_entry(string file_name, FileControlBlock* fcb);
    SystemFileEntry* get_entry(string file_name);
    bool check_entry(string file_name);
    void erase_entry(string file_name);

private:
    map<string, SystemFileEntry*> open_files;
};
