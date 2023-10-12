/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-10-12 12:51:38
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-12 14:01:51
 */
#include <iostream>
#include <string>
#include <string.h>
#include <ctime>
#include <iomanip>
#include <thread>
#include "netlib/netlib.h"
using namespace std;
#define SERVER_IP "192.168.124.140"
#define SERVER_PORT 9999
#define QUESTION_NUM 1000
#define VERSION_LEN 4
#define THREAD_NUM 4
class Proto_Head
{
public:
    Proto_Head(){
        memset(version,0,VERSION_LEN);
    }
    char version[VERSION_LEN];
    uint32_t length;
};


void Send_Msg(Client_Sock&client, string cmd)
{
    char *buffer = new char[sizeof(Proto_Head)];
    memset(buffer,0,sizeof(Proto_Head));
    Proto_Head head;
    head.length = cmd.length();
    memcpy(head.version,"1.0",VERSION_LEN);
    memcpy(buffer,&head,sizeof(Proto_Head));
    string msg;
    msg.assign(buffer,sizeof(Proto_Head));
    msg += cmd;
    client.Send(msg);
}

void Recv_Msg(Client_Sock& client)
{
    string buffer = "";
    client.Recv(buffer,sizeof(Proto_Head));
    Proto_Head head;
    memcpy(&head,buffer.c_str(),sizeof(Proto_Head));
    client.Recv(buffer,head.length);
}

void Work()
{
    Client_Sock client;
    int fd = client.Sock();
    int ret = client.Connect(SERVER_IP,SERVER_PORT);
    for(int i=0;i<QUESTION_NUM ; i++)
    {
        string cmd = "SET num "+std::to_string(i);
        Send_Msg(client,cmd);
        Recv_Msg(client);
    }
    client.Close();
}

int main()
{
    thread ths[THREAD_NUM];
    time_t beg = time(0);
    for(int i=0;i<THREAD_NUM;i++)
    {
        ths[i] = thread(Work);
    }
    for(int i=0;i<THREAD_NUM;i++)
    {
        ths[i].join();
    }
    time_t end = time(0);
    std::cout << "during : "<<end-beg<<"  s" <<std::endl;
    std::cout << "qps : " << std::fixed << std::setprecision(4) << 1.0*QUESTION_NUM*THREAD_NUM/(end-beg) <<std::endl;
    return 0;
}