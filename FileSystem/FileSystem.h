#pragma once

#include "FCB.h"
#include "ptOFT.h"
#include "swOFT.h"
#include "SS.h"
#include "VCB.h"
#include "Directory.h"

class FileSystem {
public:
    FileSystem();
    ~FileSystem();
    bool create(string file_name, int file_size);
    ptOFT_Entry* open(string file_name);
    bool close(string file_name, ptOFT* pt_table);
    char* read(string file_name, ptOFT* pt_table);                                                                                                                                                             
    bool write(string file_name, ptOFT* pt_table, char* data, int num_bytes);
    bool rename(string old_name, string new_name);
    bool remove(string file_name);

private:
    Directory dir;
    VCB vcb;
    SS storage;
    swOFT sw_table;
};                    