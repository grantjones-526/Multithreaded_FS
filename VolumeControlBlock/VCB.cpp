#include "VCB.h"

#include <vector>

// Set up a disk with 512 blocks of 2KB each.
// Block 0 is reserved for the volume control block itself,
// so 511 blocks are available for file data.
VolumeControlBlock::VolumeControlBlock() {
    this->num_blocks = 512;
    this->block_size = 2048;
    this->free_blocks = 511;
    this->bitmap.assign(num_blocks, false);
    this->bitmap[0] = true; // block 0 is reserved
}

VolumeControlBlock::~VolumeControlBlock() {}

// Search for 'count' free blocks in a row on disk.
// If found, mark them all as used and return the starting block number.
// If there is not enough contiguous space, return -1.
int VolumeControlBlock::get_contiguous_blocks(int count) {
    for (int i = 0; i <= num_blocks - count; i++) {
        bool found = true;
        for (int j = i; j < i + count; j++) {
            if (bitmap[j] == true) {
                found = false;
                i = j; // skip ahead past the used block
                break;
            }
        }
        if (found) {
            // Mark all blocks in this range as used
            for (int j = i; j < i + count; j++) {
                bitmap[j] = true;
            }
            free_blocks -= count;
            return i;
        }
    }
    return -1;
}

// Mark 'count' blocks starting at 'start' as free again.
void VolumeControlBlock::free_space(int start, int count) {
    for (int i = start; i < start + count; i++) {
        bitmap[i] = false;
        free_blocks++;
    }
}

// Mark a single block as used.
void VolumeControlBlock::fill_block(int block_index, int file_size) {
    if (block_index < this->num_blocks) {
        this->bitmap[block_index] = true;
        this->free_blocks--;
    }
}

int VolumeControlBlock::get_num_blocks() {
    return this->num_blocks;
}

int VolumeControlBlock::get_block_size() {
    return this->block_size;
}

// Find the first single free block, mark it as used, and return its index.
// Returns -1 if the disk is completely full.
int VolumeControlBlock::get_free_block() {
    for (int i = 0; i < num_blocks; i++) {
        if (bitmap[i] == false) {
            bitmap[i] = true;
            free_blocks--;
            return i;
        }
    }
    return -1;
}

vector<bool> VolumeControlBlock::get_bitmap() {
    return this->bitmap;
}
