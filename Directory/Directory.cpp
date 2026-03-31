#include "Directory.h"


Directory::Directory() {}

void Directory::add_entry(string file_name, FCB* entry) {
    this->directory_table.insert({file_name, entry});
}

FCB* Directory::get_entry(string file_name) {
    if (!Directory::check_entry(file_name)){
        return nullptr;
    }
    return this->directory_table.at(file_name);
}

bool Directory::check_entry(string file_name) {
    return this->directory_table.count(file_name) > 0;
}

void Directory::erase_entry(string file_name) {
    this->directory_table.erase(file_name);
}

Directory::~Directory() {}