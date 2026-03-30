#include "VCB.h"

#include <vector>
using namespace std;

VCB::VCB() {
    //default sizes can be adjusted here
    this->num_blocks = 512;
    this->size_block = 2000;
    this->free_blocks = 512; //decrease by one when a block is filled
    this->bitmap.assign(num_blocks, false);
}

VCB::~VCB() {

}

int VCB::get_num_blocks(){
    return this->num_blocks;
}

int VCB::get_size_block(){
    return this->size_block;
}

int VCB::get_free_block(){
    // go through bitmap to find first free space
    for (int i = 0; i < num_blocks; i++) {
        if (bitmap[i] == 0){
            return i;
        }
    }
    return -1;
}

vector<bool> VCB::get_bitmap(){
    return this->bitmap;
}