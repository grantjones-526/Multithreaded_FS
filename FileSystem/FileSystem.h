#pragma once

#include "FCB.h"
#include "ptOFT.h"
#include "swOFT.h"
#include "SS.h"
#include "VCB.h"
#include "Directory.h"
#include <pthread.h>

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

    // Fine-grained locks: one mutex per shared data structure
    // Lock ordering (to prevent deadlock): dir -> swoft -> vcb -> storage
    pthread_mutex_t dir_mutex;      // protects directory structure
    pthread_mutex_t swoft_mutex;    // protects system-wide open file table + reference counts
    pthread_mutex_t vcb_mutex;      // protects free space / block allocation
    pthread_mutex_t storage_mutex;  // protects simulated disk storage
};                    