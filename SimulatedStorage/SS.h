#pragma once

class SS {
public:
    SS();
    ~SS();
    void read_block(int block_index, char* buffer);
    void write_block(int block_index, char* data);
private:
    static const int BLOCK_SIZE = 2048;
    static const int NUM_BLOCKS = 512;
    char* storage;
};