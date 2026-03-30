#pragma once

class FCB
{
public:
    FCB();
    FCB(int start_block, int file_size);
    ~FCB();
    int get_file_size();
    int get_start_block();

private:
    int file_size;
    int start_block;
};