#include "FCB.h"

FCB::FCB() {
    this->file_size = 0;
    this->start_block_pointer = -1;
}

FCB::FCB(int start_block, int file_size) {
    this->start_block_pointer = start_block; //would the start_block not just be set from the first available free block found in vcb (maybe call vcb to get location)
    this->file_size = file_size;
}

int FCB::get_file_size() {
    return this->file_size;
}

int FCB::get_start_block() {
    return this->start_block_pointer;
}

FCB::~FCB() {}
