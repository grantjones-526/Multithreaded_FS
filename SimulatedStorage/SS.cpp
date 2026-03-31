#include "SS.h"
#include <cstring>

SS::SS() {
    storage = new char[NUM_BLOCKS * BLOCK_SIZE]();
}

SS::~SS() {
    delete[] storage;
}

void SS::read_block(int block_index, char* buffer) {
    memcpy(buffer, &storage[block_index * BLOCK_SIZE], BLOCK_SIZE);
}

void SS::write_block(int block_index, char* data) {
    memcpy(&storage[block_index * BLOCK_SIZE], data, BLOCK_SIZE);
}