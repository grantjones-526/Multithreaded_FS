#pragma once

#include "FCB.h"
#include "ptOFT.h"
#include "swOFT.h"
#include "SS.h"
#include "VCB.h"
#include "Directory.h"
#include <pthread.h>

// The main file system class.
// Provides create, open, close, read, write, rename, and remove operations.
// All operations are thread-safe using mutex locks.
class FileSystem {
public:
    FileSystem();
    ~FileSystem();

    bool create(string file_name, int file_size);
    ThreadFileEntry* open(string file_name, string mode = "rw");
    bool close(string file_name, ThreadOpenFileTable* thread_table);
    char* read(string file_name, ThreadOpenFileTable* thread_table);
    bool write(string file_name, ThreadOpenFileTable* thread_table, char* data, int num_bytes);
    bool rename(string old_name, string new_name);
    bool remove(string file_name);

private:
    Directory directory;
    VolumeControlBlock volume;
    DiskStorage disk;
    SystemOpenFileTable system_file_table;

    // One lock per shared data structure.
    // Lock ordering (to prevent deadlock):
    //   directory_lock -> file_table_lock -> volume_lock -> disk_lock
    pthread_mutex_t directory_lock;    // protects the directory
    pthread_mutex_t file_table_lock;   // protects the system-wide open file table
    pthread_mutex_t volume_lock;       // protects block allocation (volume control block)
    pthread_mutex_t disk_lock;         // protects the simulated disk storage
};
