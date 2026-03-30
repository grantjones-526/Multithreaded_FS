#include "FileSystem.h"

#include <cstring>

FileSystem::FileSystem() {}

bool FileSystem::create(string file_name, int file_size) {
    if (this->dir.check_entry(file_name)) {
        return 0;
    }

    int free_block = this->vcb.get_contiguous_blocks(file_size);
    if (free_block == -1) {
        return 0;
    }

    FCB* entry = new FCB(free_block, file_size);
    this->dir.add_entry(file_name, entry);
    return 1;
}

 ptOFT_Entry* FileSystem::open(string file_name) {
    FCB* fcb = this->dir.get_entry(file_name);
    if (fcb == nullptr) {                                                                                                                                                                   
        return nullptr;
    }                                                                                                                                                                                       
                
    swOFT_Entry* sw_entry;                                                                                                                                                                  
    if (this->sw_table.check_entry(file_name)) {
        sw_entry = this->sw_table.get_entry(file_name);                                                                                                                                     
        sw_entry->reference_count++;
    } else {                                                                                                                                                                                
        sw_entry = this->sw_table.add_entry(file_name, fcb);
    }                                                                                                                                                                                       
                
    // create and return per-thread entry                                                                                                                                                   
    // (the ptOFT table would live per-thread, not here)
    ptOFT_Entry* pt_entry = new ptOFT_Entry(0, sw_entry);                                                                                                                                   
    return pt_entry; 
}

bool FileSystem::close(string file_name, ptOFT* pt_table) {                                                                                                                                 
    ptOFT_Entry* pt_entry = pt_table->get_entry(file_name);
    if (pt_entry == nullptr) {                                                                                                                                                              
        return 0;
    }                                                                                                                                                                                       
                
    swOFT_Entry* sw_entry = pt_entry->swOFT_pointer;                                                                                                                                        
    sw_entry->reference_count--;
                                                                                                                                                                                            
    if (sw_entry->reference_count == 0) {
        this->sw_table.erase_entry(file_name);
    }                                                                                                                                                                                       

    pt_table->erase_entry(file_name);                                                                                                                                                       
    return 1;   
}

char* FileSystem::read(string file_name, ptOFT* pt_table) {                                                                                                                                 
    ptOFT_Entry* pt_entry = pt_table->get_entry(file_name);                                                                                                                                 
    if (pt_entry == nullptr) {                                                                                                                                                              
        return nullptr;
    }                                                                                                                                                                                       
                
    swOFT_Entry* sw_entry = pt_entry->swOFT_pointer;
    FCB* fcb = sw_entry->fcb_pointer;

    int start_block = fcb->get_start_block();                                                                                                                                               
    int file_size = fcb->get_file_size();
                                                                                                                                                                                            
    char* address = this->storage.get_address(start_block);
    char* buffer = new char[file_size];
    memcpy(buffer, address, file_size);                                                                                                                                                     

    return buffer;                                                                                                                                                                          
} 

bool FileSystem::write(string file_name, ptOFT* pt_table, char* data, int num_bytes) {                                                                                                      
    ptOFT_Entry* pt_entry = pt_table->get_entry(file_name);                                                                                                                                 
    if (pt_entry == nullptr) {                                                                                                                                                              
        return 0;
    }                                                                                                                                                                                       
                
    swOFT_Entry* sw_entry = pt_entry->swOFT_pointer;
    FCB* fcb = sw_entry->fcb_pointer;

    int start_block = fcb->get_start_block();                                                                                                                                               
    int offset = sw_entry->file_offset;
                                                                                                                                                                                            
    char* address = this->storage.get_address(start_block);                                                                                                                                 
    memcpy(address + offset, data, num_bytes);
                                                                                                                                                                                            
    sw_entry->file_offset += num_bytes;
    return 1;
}

bool FileSystem::rename(string old_name, string new_name) {                                                                                                                                 
    if (!this->dir.check_entry(old_name)) {                                                                                                                                                 
        return 0;                                                                                                                                                                           
    }                                                                                                                                                                                       
    if (this->dir.check_entry(new_name)) {                                                                                                                                                  
        return 0;
    }

    FCB* fcb = this->dir.get_entry(old_name);                                                                                                                                               
    this->dir.add_entry(new_name, fcb);
    this->dir.erase_entry(old_name);                                                                                                                                                        
                                                                                                                                                                                            
    return 1;
}   

bool FileSystem::remove(string file_name) {                                                                                                                                                 
    if (!this->dir.check_entry(file_name)) {                                                                                                                                                
        return 0;                                                                                                                                                                           
    }                                                                                                                                                                                       
                                                                                                                                                                                            
    if (this->sw_table.check_entry(file_name)) {                                                                                                                                            
        return 0; // can't delete while file is open
    }                                                                                                                                                                                       
                
    FCB* fcb = this->dir.get_entry(file_name);                                                                                                                                              
    int start_block = fcb->get_start_block();
    int file_size = fcb->get_file_size();                                                                                                                                                   
                
    // free the blocks in VCB                                                                                                                                                               
    this->vcb.free_space(start_block, file_size);
                                                                                                                                                                                            
    this->dir.erase_entry(file_name);                                                                                                                                                       
    delete fcb;
                                                                                                                                                                                            
    return 1;   
}

FileSystem::~FileSystem() {

}