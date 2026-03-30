#include "SS.h"

SS::SS() {
    
}
char* SS::get_address(int block_num) {
    return &this->storage[block_num * 2048];
}

SS::~SS() {

}