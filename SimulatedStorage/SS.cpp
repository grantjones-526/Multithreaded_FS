#include "SS.h"
#include <cstring>

// Allocate 1MB of memory to simulate a disk. All bytes start as zero.
DiskStorage::DiskStorage() {
    storage = new char[NUM_BLOCKS * BLOCK_SIZE]();
}

DiskStorage::~DiskStorage() {
    delete[] storage;
}

// Copy one 2KB block from the simulated disk into the given buffer.
void DiskStorage::read_block(int block_index, char* buffer) {
    memcpy(buffer, &storage[block_index * BLOCK_SIZE], BLOCK_SIZE);
}

// Copy the given data into one 2KB block on the simulated disk.
void DiskStorage::write_block(int block_index, char* data) {
    memcpy(&storage[block_index * BLOCK_SIZE], data, BLOCK_SIZE);
}
