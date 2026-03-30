#pragma once

#include "FCB.h"
#include <string>

using namespace std;

class swOFT {
public:
    swOFT();
    ~swOFT();

private:
    string file_name;
    int file_offset;
    int reference_count;
    string access_mode;
    FCB* fcb_pointer = nullptr;
};