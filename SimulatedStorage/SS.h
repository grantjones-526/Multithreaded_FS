#pragma once

// Represents the physical disk as a 1MB block of memory.
// Data is read and written one block (2KB) at a time.
class DiskStorage {
public:
    DiskStorage();
    ~DiskStorage();
    void read_block(int block_index, char* buffer);
    void write_block(int block_index, char* data);

private:
    static const int BLOCK_SIZE = 2048;   // 2KB per block
    static const int NUM_BLOCKS = 512;    // 512 blocks total = 1MB
    char* storage;                        // the raw byte array (simulated disk)
};
