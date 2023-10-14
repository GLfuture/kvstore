/*
 * @Description: 
 * @Version: 4.9
 * @Author: Gong
 * @Date: 2023-09-30 11:59:38
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-13 11:30:45
 */
#pragma once
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <map>
#include <memory>
#include <mutex>
#include "conn.h"
using std::map;
using std::shared_ptr;
enum Error_Code
{
    OK = 0,
    SOCKET_ERR = -1,
    CONNECT_ERR = -2,
};

class Server_Base
{
public:
    using Tcp_Conn_Base_Ptr = shared_ptr<Tcp_Conn_Base>;
    Server_Base();

    int Conncet(string sip,uint32_t sport);

    int Bind(uint32_t port);

    int Listen(uint32_t backlog);

    int Accept();

    ssize_t Recv(const Tcp_Conn_Base_Ptr& conn_ptr,uint32_t len);

    ssize_t Send(const Tcp_Conn_Base_Ptr& conn_ptr,uint32_t len);

    Tcp_Conn_Base_Ptr Get_Conn(int fd) { return connections[fd]; }

    void Add_Conn(const Tcp_Conn_Base_Ptr& conn_ptr);

    map<uint32_t, Tcp_Conn_Base_Ptr>::iterator Del_Conn(int fd);

    size_t Get_Conn_Num() { return connections.size(); }

    int Close(int fd);

    void Clean_Conns();

    const char* Get_Ret_Str(Error_Code code){
        const char *arr[] =
            {
                "successful",
                "socket function is failed",
                "connect function is failed",
            };
        return arr[code];
    }

    int Get_Sock() { return _fd; }

    virtual ~Server_Base() {
        
    }
    
protected:
    std::mutex mtx;
    int _fd;
    map<uint32_t,Tcp_Conn_Base_Ptr> connections;
};