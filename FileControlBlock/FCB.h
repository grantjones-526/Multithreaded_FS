#pragma once

// Represents a single file's metadata on disk.
// Stores where the file starts and how many blocks it uses.
class FileControlBlock
{
public:
    FileControlBlock();
    FileControlBlock(int start_block, int file_size);
    ~FileControlBlock();
    int get_file_size();
    int get_start_block();

private:
    int file_size;       // number of blocks this file occupies
    int start_block;     // first block on disk where this file's data begins
};
