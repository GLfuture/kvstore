/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-10-10 12:14:40
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-12 12:56:22
 */
#pragma once
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <unistd.h>

class Client_Sock_Base
{
public:
    virtual int Sock() = 0;
    virtual int Connect(const std::string ip , const uint16_t port) = 0;
    virtual ssize_t Send(const std::string buffer) = 0;
    virtual ssize_t Recv(std::string& buffer,ssize_t len) = 0;
    virtual int Close() = 0;
protected:
    int fd;
};


class Client_Sock: public Client_Sock_Base
{
public:
    int Sock() override
    {
        fd = socket(AF_INET,SOCK_STREAM,0);
        return fd;
    }

    int Connect(const std::string ip , const uint16_t port) override
    {
        sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        return connect(fd,(sockaddr*)&addr,sizeof(addr));

    }

    ssize_t Send(const std::string buffer)
    {
        return send(fd,buffer.c_str(),buffer.length(),0);
    }

    ssize_t Recv(std::string& buffer,ssize_t len)
    {
        char* temp = new char[len];
        memset(temp,0,len);
        ssize_t ret = recv(fd,temp,len,0);
        buffer.assign(temp,ret);
        return ret;
    }


    int Close()
    {
        return close(fd);
    }
};