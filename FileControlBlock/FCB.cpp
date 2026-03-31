#include "FCB.h"

// Default constructor -- no file assigned yet.
FileControlBlock::FileControlBlock() {
    this->file_size = 0;
    this->start_block = -1;
}

// Create a file control block for a file starting at 'start_block'
// and occupying 'file_size' contiguous blocks.
FileControlBlock::FileControlBlock(int start_block, int file_size) {
    this->start_block = start_block;
    this->file_size = file_size;
}

// Returns how many blocks this file uses.
int FileControlBlock::get_file_size() {
    return this->file_size;
}

// Returns the first block number where this file's data is stored.
int FileControlBlock::get_start_block() {
    return this->start_block;
}

FileControlBlock::~FileControlBlock() {}
