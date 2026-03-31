#include "VCB.h"

#include <vector>

VCB::VCB() {
    //default sizes can be adjusted here
    this->num_blocks = 512;
    this->size_block = 2048;
    this->free_blocks = 511; // block 0 reserved for VCB
    this->bitmap.assign(num_blocks, false);
    this->bitmap[0] = true; // reserve block 0 for VCB
}

VCB::~VCB() {}

//this doesnt make sense... you would only get contiguous blocks based on file size from fcb. no need to pass in count, just get blocks based on filesize
int VCB::get_contiguous_blocks(int count) {                                                                                                                                                 
    for (int i = 0; i <= num_blocks - count; i++) {                                                                                                                                         
        bool found = true;                                                                                                                                                                  
        for (int j = i; j < i + count; j++) {                                                                                                                                               
            if (bitmap[j] == 1) {                                                                                                                                                           
                found = false;                                                                                                                                                              
                i = j; // skip ahead
                break;                                                                                                                                                                      
            }   
        }                                                                                                                                                                                   
        if (found) {
            for (int j = i; j < i + count; j++) {                                                                                                                                           
                bitmap[j] = 1;
            }
            free_blocks -= count;
            return i; // return starting block
        }                                                                                                                                                                                   
    }
    return -1; // not enough contiguous space                                                                                                                                               
}

//free space should use values provided by fcb (filesize and start block pointer)
void VCB::free_space(int start, int count) {                                                                                                                                               
    for (int i = start; i < start + count; i++) {
        bitmap[i] = 0;
        free_blocks++;                                                                                                                                                                      
    }
}  

void VCB::fill_block(int block_index, int file_size) {
    // mark block as filled in bitmap
    if (block_index < this->num_blocks) {
        this->bitmap[block_index] = true;
        this->free_blocks--;
    }
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
            bitmap[i] = 1; // mark block as used
            free_blocks--;
            return i;
        }
    }
    return -1;
}

vector<bool> VCB::get_bitmap(){
    return this->bitmap;
}