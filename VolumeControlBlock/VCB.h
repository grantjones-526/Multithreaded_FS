#pragma once

#include <vector>
using namespace std;

class VCB {
public:
    VCB();
    ~VCB();
    int get_num_blocks();
    int get_contiguous_blocks(int count);
    int get_size_block();
    int get_free_block();
    void fill_block(int block_index, int file_size);
    void free_space(int start, int count);
    vector<bool> get_bitmap();

private:
    int num_blocks;
    int size_block;
    int free_blocks;
    vector<bool> bitmap;
};