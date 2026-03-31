#pragma once

#include "FCB.h"
#include <string>
#include <map>

using namespace std;

// The flat directory -- maps file names to their file control blocks.
// No subdirectories; all files exist at one level.
class Directory {
public:
    Directory();
    ~Directory();
    void add_entry(string file_name, FileControlBlock* entry);
    FileControlBlock* get_entry(string file_name);
    bool check_entry(string file_name);
    void erase_entry(string file_name);

private:
    map<string, FileControlBlock*> files; // file name (key) -> file control block
};
