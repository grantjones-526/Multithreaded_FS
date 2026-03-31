#pragma once

#include "swOFT.h"
#include <map>
#include <string>

using namespace std;

// One entry in a thread's private open file table.
// Each thread gets its own entry (with its own file offset) for each file it opens.
class ThreadFileEntry {
public:
    ThreadFileEntry(int handle, SystemFileEntry* system_entry);
    ~ThreadFileEntry();

    int handle;                          // this thread's file descriptor number
    int file_offset;                     // where this thread will read/write next (in bytes)
    SystemFileEntry* system_entry;       // pointer to the shared system-wide entry
};

// A thread's private open file table.
// Each thread has one of these to track which files it has open.
class ThreadOpenFileTable {
public:
    ThreadOpenFileTable();
    ~ThreadOpenFileTable();
    ThreadFileEntry* add_entry(string file_name, SystemFileEntry* system_entry);
    ThreadFileEntry* get_entry(string file_name);
    bool check_entry(string file_name);
    void erase_entry(string file_name);

private:
    map<string, ThreadFileEntry*> open_files;
    int next_handle = 0;  // auto-incrementing file descriptor number
};
