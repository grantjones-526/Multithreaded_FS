#include "ptOFT.h"

#include <string>

using namespace std;

ptOFT::ptOFT() {

}

string ptOFT::get_file_name() {
    return this->file_name;
}

int ptOFT::get_handle() {
    return this->handle;
}

ptOFT::~ptOFT() {

}