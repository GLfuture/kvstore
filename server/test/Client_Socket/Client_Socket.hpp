#pragma once
#ifndef CLIENT_SOCKET
#define CLIENT_SOCKET
#endif
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string_view>
#include <string>
#include <string.h>
#include <unistd.h>
namespace Client_Socket_NSP
{
#define DEFAULT_PORT 9898
#define DEFAUL_BUFFER_SIZE 1024
#define DEFAULT_SIZE -1
    using CString = std::string_view;
    using std::string;

    class Client_Socket
    {
    public:
        Client_Socket(string sip, uint16_t sport, uint32_t buffersize = DEFAUL_BUFFER_SIZE) : _server_ip(sip), _server_port(sport), _buffersize(buffersize)
        {
            this->_buffer = new char[buffersize];
            memset(this->_buffer, 0, buffersize);
        }

        int Socket_Init(int port = DEFAULT_PORT)
        {
            this->client_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (this->client_fd <= 0)
            {
                return client_fd;
            }
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            int ret = bind(client_fd, (sockaddr *)&addr, sizeof(addr));
            return ret;
        }

        int Connect()
        {
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(this->_server_port);
            addr.sin_addr.s_addr = inet_addr(_server_ip.c_str());
            int ret = connect(client_fd, (sockaddr *)&addr, sizeof(addr));
            return ret;
        }
        // 接收指定长度内容先保存至缓冲区(注意缓冲区大小)，然后拷贝一份到rbuffer并清空缓冲区
        // 默认接收_buffersize长度
        int Recv(uint32_t size = DEFAULT_SIZE)
        {
            if (size == DEFAULT_SIZE)
                size = this->_buffersize;
            int len = recv(this->client_fd, this->_buffer, size, 0);
            this->rbuffer += this->_buffer;
            memset(this->_buffer, 0, this->_buffersize);
            return len;
        }

        int Close()
        {
            return close(client_fd);
        }

        // 发送wbuffer中的内容并清空wbuffer已发送的内容
        // 默认发送wbuffer中的所有内容
        int Send(uint32_t size = DEFAULT_SIZE)
        {
            if (size == DEFAULT_SIZE)
                size = this->wbuffer.length();
            int len = send(this->client_fd, wbuffer.c_str(), size, 0);
            Erase(len);
            return len;
        }
        // 清空读缓冲区
        void Clean_Read_Buffer()
        {
            this->rbuffer.clear();
        }
        // 清空写缓冲区
        void Clean_Send_Buffer()
        {
            this->wbuffer.clear();
        }

        CString Get_Read_Buffer()
        {
            return this->rbuffer;
        }

        CString Get_Server_Ip()
        {
            return _server_ip;
        }

        uint16_t Get_Server_Port()
        {
            return _server_port;
        }

        uint32_t Get_Buffer_Size()
        {
            return this->_buffersize;
        }

        void Set_Server_Ip(CString ip)
        {
            this->_server_ip = ip;
        }

        void Set_Server_Port(uint16_t port)
        {
            this->_server_port = port;
        }

        void Set_Write_Buffer(CString buffer)
        {
            this->wbuffer.assign(buffer.cbegin(), buffer.cend());
        }

        void Set_Buffer_Size(uint32_t size)
        {
            this->_buffersize = size;
            delete this->_buffer;
            this->_buffer = new char[size];
            memset(this->_buffer, 0, size);
        }

    private:
        void Erase(int len)
        {
            string::iterator it = this->wbuffer.begin();
            for (int i = 0; i < len; i++)
            {
                it++;
            }
            this->wbuffer.erase(this->wbuffer.begin(), it);
        }

    private:
        int client_fd;
        string _server_ip;     // sever端ip
        uint16_t _server_port; // server端port
        string wbuffer;
        string rbuffer;
        char *_buffer;
        uint32_t _buffersize;
    };
}