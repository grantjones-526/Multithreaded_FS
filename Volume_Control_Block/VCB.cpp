#include "VCB.h"

#include <vector>
using namespace std;

VCB::VCB() {
    //default sizes can be adjusted here
    this->numBlocks = 512;
    this->sizeBlock = 2000;
    this->freeBlock = 512;
    this->bitmap.assign(numBlocks, false);
}

VCB::~VCB() {

}

int VCB::getnumBlocks(){
    return this->numBlocks;
}

int VCB::getsizeBlock(){
    return this->sizeBlock;
}

int VCB::getfreeBlock(){
    // go through bitmap to find first free space
    for (int i = 0; i < numBlocks; i++) {
        if (bitmap[i] == 0){
            return i;
        }
    }
    return -1;
}

vector<bool> VCB::getbitmap(){
    return this->bitmap;
}