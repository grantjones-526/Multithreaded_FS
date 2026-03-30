#include "ptOFT.h"
                
ptOFT_Entry::ptOFT_Entry(int handle, swOFT_Entry* sw_entry) {                                                                                                                               
    this->handle = handle;                                   
    this->swOFT_pointer = sw_entry;                                                                                                                                                         
}                                  
                                                                                                                                                                                            
ptOFT_Entry::~ptOFT_Entry() {}
                                                                                                                                                                                            
ptOFT::ptOFT() {}
                
ptOFT_Entry* ptOFT::add_entry(string file_name, swOFT_Entry* sw_entry) {
    ptOFT_Entry* entry = new ptOFT_Entry(this->next_handle, sw_entry);                                                                                                                      
    this->ptOFT_map[file_name] = entry;                               
    this->next_handle++;                                                                                                                                                                    
    return entry;                                                                                                                                                                           
}                
                                                                                                                                                                                            
ptOFT_Entry* ptOFT::get_entry(string file_name) {
    if (!ptOFT::check_entry(file_name)) {        
        return nullptr;                  
    }                  
    return this->ptOFT_map.at(file_name);                                                                                                                                                   
}                                        
                                                                                                                                                                                            
bool ptOFT::check_entry(string file_name) {
    return this->ptOFT_map.count(file_name) > 0;
}                                               
                                                                                                                                                                                            
void ptOFT::erase_entry(string file_name) {
    this->ptOFT_map.erase(file_name);                                                                                                                                                       
}                                    

ptOFT::~ptOFT() {}
