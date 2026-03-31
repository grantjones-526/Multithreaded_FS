#pragma once

#include <vector>
using namespace std;

// Manages the disk's block allocation using a bitmap.
// Each bit tracks whether a block is free (0) or used (1).
class VolumeControlBlock {
public:
    VolumeControlBlock();
    ~VolumeControlBlock();
    int get_num_blocks();
    int get_block_size();
    int get_free_block();
    int get_contiguous_blocks(int count);
    void fill_block(int block_index, int file_size);
    void free_space(int start, int count);
    vector<bool> get_bitmap();

private:
    int num_blocks;      // total number of blocks on disk (512)
    int block_size;      // size of each block in bytes (2048)
    int free_blocks;     // how many blocks are currently available
    vector<bool> bitmap; // true = used, false = free
};
