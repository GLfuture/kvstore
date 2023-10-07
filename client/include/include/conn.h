/*
 * @Description: 
 * @Version: 4.9
 * @Author: Gong
 * @Date: 2023-09-30 11:59:38
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-03 12:53:18
 */
#pragma once
#ifndef CONN_H
#define CONN_H
#endif
#include <iostream>
#include <string>
#include <string.h>
#include <string_view>
using std::string;
using std::string_view;
class Tcp_Conn_Base
{
public:
    Tcp_Conn_Base(uint32_t conn_fd);

    size_t Get_Rbuffer_Length() { return _rbuffer.length(); }

    size_t Get_Wbuffer_Length() { return _wbuffer.length(); }

    void Appand_Rbuffer(string rbuffer) { this->_rbuffer.append(rbuffer); }

    void Appand_Wbuffer(string wbuffer) { this->_wbuffer.append(wbuffer); }

    // 只提供视图，以方便粘包拷贝不丢失数据
    string_view Get_Rbuffer() { return _rbuffer; }

    string_view Get_Wbuffer() { return _wbuffer; }
    // 配合Get_Rbuffer解决粘包的问题(在业务中只删除一个包的数据)
    void Erase_Rbuffer(int len);

    void Erase_Wbuffer(int len);

    uint32_t Get_Conn_fd() { return _fd; }

    virtual ~Tcp_Conn_Base(){
        
    }
protected:
    uint32_t _fd;
    string _rbuffer;
    string _wbuffer;
};

class Tcp_Conn:public Tcp_Conn_Base
{
public:
    Tcp_Conn(uint32_t conn_fd):Tcp_Conn_Base(conn_fd){
        
    }
    ~Tcp_Conn() override{
        
    }
};