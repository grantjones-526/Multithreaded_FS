#pragma once                                                                                                                                                                             
                                                                                                                                                                                            
#include "swOFT.h"                                                                                                                                                                          
#include <map>    
#include <string>
                
using namespace std;                                                                                                                                                                        
                    
class ptOFT_Entry {                                                                                                                                                                         
public:            
    ptOFT_Entry(int handle, swOFT_Entry* sw_entry);
    ~ptOFT_Entry();                                
    int handle;    
    swOFT_Entry* swOFT_pointer;
};                             
                                                                                                                                                                                            
class ptOFT {
public:                                                                                                                                                                                     
    ptOFT();    
    ~ptOFT();
    ptOFT_Entry* add_entry(string file_name, swOFT_Entry* sw_entry);
    ptOFT_Entry* get_entry(string file_name);                       
    bool check_entry(string file_name);                                                                                                                                                     
    void erase_entry(string file_name);
private:                                                                                                                                                                                    
    map<string, ptOFT_Entry*> ptOFT_map;
    int next_handle = 0;
};                         