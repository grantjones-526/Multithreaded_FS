#pragma once

class SS {
public:
    SS();
    ~SS();
    char* get_address(int block_num);
private:
    char storage[512 * 2048] = {0};
};