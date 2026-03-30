#include "FCB.h"

using namespace std;

FCB::FCB() {
    this->file_size = 0;
    //first_data_block points to first data block in ss
}
FCB::FCB(int* first_datablock) {
    this->first_datablock = first_datablock;
}

int FCB::get_file_size() {
    return this->file_size;
}

int* FCB::get_first_datablock() {
    return this->first_datablock;
}

FCB::~FCB() {

}