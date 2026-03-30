#pragma once

#include "swOFT.h"
#include <string>

using namespace std;

class ptOFT {
public:
    ptOFT();
    ~ptOFT();
    string get_file_name();
    int get_handle();

private:
    string file_name;
    int handle;
    swOFT* ptoft_pointer = nullptr;
};