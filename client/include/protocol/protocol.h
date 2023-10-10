/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-10-10 12:54:19
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-10 12:54:38
 */
#pragma
#include <iostream>
#include <string.h>
#define VERSION_LEN 4

class Proto_Head
{
public:
    Proto_Head(){
        memset(version,0,VERSION_LEN);
    }
    char version[VERSION_LEN];
    uint32_t length;
};
