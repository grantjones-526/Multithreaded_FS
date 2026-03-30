#pragma once

class FCB
{
public:
    FCB();
    FCB(int* first_datablock);
    ~FCB();
    int get_file_size();
    int* get_first_datablock();

private:
    int file_size;
    int* first_datablock = nullptr;
};