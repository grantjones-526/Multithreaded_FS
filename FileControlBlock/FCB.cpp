#include "FCB.h"

FCB::FCB() {
    this->file_size = 0;
    this->start_block = -1;
}

FCB::FCB(int start_block, int file_size) {
    this->start_block = start_block;
    this->file_size = file_size;
}

int FCB::get_file_size() {
    return this->file_size;
}

int FCB::get_start_block() {
    return this->start_block;
}

FCB::~FCB() {}
