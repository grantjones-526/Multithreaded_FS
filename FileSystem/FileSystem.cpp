#include "FileSystem.h"

FileSystem::FileSystem() {

}

bool FileSystem::create(string file_name, int file_size) {
    if (this->dir.check_entry(file_name)) {
        return 0;
    }
    int free_block = this->vcb.get_free_block();
    FCB entry = new FCB(free_block);
}

bool FileSystem::open(string file_name) {
    FCB* fcb_entry = nullptr;
    fcb_entry = this->dir.get_entry(file_name);
}

bool FileSystem::close() {

}
bool FileSystem::read() {

}
bool FileSystem::write() {

}
bool FileSystem::rename() {

}
bool FileSystem::remove() {

}

bool FileSystem::~FileSystem() {

}