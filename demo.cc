/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-10-03 03:55:23
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-03 09:09:05
 */
#include <iostream>
#include <map>
#include <string>
#include <string.h>
using namespace std;
int main()
{
    long a = 10;
    char* buffer = new char[8];
    memset(buffer,0,8);
    memcpy(buffer,&a,8);
    string temp(buffer);
    long b;
    memcpy(&b,temp.c_str(),8);
    std::cout<<b<<std::endl;
    return 0;
}