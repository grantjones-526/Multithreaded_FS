#pragma once

#include <vector>
using namespace std;

class VCB {
public:
    VCB();
    ~VCB();
    int get_num_blocks();
    int get_size_block();
    int get_free_block();
    vector<bool> get_bitmap();

private:
    int num_blocks;
    int size_block;
    int free_blocks;
    vector<bool> bitmap;
};