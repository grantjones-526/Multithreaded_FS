#include "swOFT.h"

using namespace std;

swOFT_Entry::swOFT_Entry(FCB* fcb) {
    this->fcb_pointer = fcb;
    this->file_offset = 0;                                                                                                                                                                  
    this->reference_count = 1;
}   

swOFT::swOFT() {}

swOFT_Entry::~swOFT_Entry() {}

swOFT::~swOFT() {}

swOFT_Entry* swOFT::add_entry(string file_name, FCB* fcb) {
    swOFT_Entry* entry = new swOFT_Entry(fcb);
    this->swOFT_map[file_name] = entry;
    return entry;
}

swOFT_Entry* swOFT::get_entry(string file_name) {
    if (!swOFT::check_entry(file_name)){
        return nullptr;
    }
    return this->swOFT_map.at(file_name);
}                                                   

bool swOFT::check_entry(string file_name) {
    return this->swOFT_map.count(file_name) > 0;
}

void swOFT::erase_entry(string file_name) {
    this->swOFT_map.erase(file_name);
}