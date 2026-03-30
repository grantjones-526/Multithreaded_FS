#pragma once

#include "FCB.h"
#include <string>
#include <map>

using namespace std;

class Directory {
public:
    Directory();
    ~Directory();
    void add_entry(string file_name, FCB* entry);
    FCB* get_entry(string file_name);
    bool check_entry(string file_name);
    void erase_entry(string file_name);
private:
    map<string, FCB*> directory_table;
};