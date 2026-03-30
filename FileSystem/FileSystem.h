#pragma once

#include "FCB.h"
#include "ptOFT.h"
#include "swOFT.h"
#include "SS.h"
#include "VCB.h"
#include "Directory.h"

class FileSystem {
public:
    FileSystem();
    ~FileSystem();

private:
    Directory dir;
    VCB vcb;
    SS storage;


};