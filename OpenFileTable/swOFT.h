#pragma once

#include "FCB.h"
#include <map>
#include <string>

using namespace std;

class swOFT_Entry {
public:
    swOFT_Entry(FCB* fcb);
    int file_offset;
    int reference_count;                                                                                                                                                                    
    string access_mode;
    FCB* fcb_pointer;
private:
    string file_name;
};

class swOFT {
public:
    swOFT();
    ~swOFT();
    swOFT_Entry* add_entry(string file_name, FCB* fcb);
    swOFT_Entry* get_entry(string file_name);                                                                                                                                               
    bool check_entry(string file_name);
    void erase_entry(string file_name);  
private:
    map<string, swOFT_Entry*> swOFT_map;
};