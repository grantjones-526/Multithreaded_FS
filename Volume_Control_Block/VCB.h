#ifndef VCB_H
#define VCB_H

#pragma once

#include <vector>
using namespace std;

class VCB
{
public:
    VCB();
    ~VCB();
    int getnumBlocks();
    int getsizeBlock();
    int getfreeBlock();
    vector<bool> getbitmap();

private:
    int numBlocks;
    int sizeBlock;
    int freeBlock;
    vector<bool> bitmap;
};

#endif