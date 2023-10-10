/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-29 05:40:03
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-10 13:52:59
 */

#include "network/netlib.hpp"
#include "config/config.h"
#include "spdlog/spdlog.h"
#include "protocol/protocol.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <string.h>
#include <algorithm>
#include <cctype>
#define MAX_COMMAND_LENGTH 64
static auto logger = spdlog::basic_logger_mt("logger","logs/info.log");
static std::string remote_addr = "127.0.0.1";
static int remote_port = 9999;

enum COMMAND{
    COMMAND_BEG = 0,
    COMMAND_QUIT = COMMAND_BEG,
    COMMAND_END,
};

const char*command[]={
    "quit"
};

void Send_Msg(Client_Sock& client , const std::string& cmd)
{
    char *buffer = new char[sizeof(Proto_Head)];
    memset(buffer,0,sizeof(Proto_Head));
    Proto_Head head ;
    head.length = cmd.length();
    memcpy(head.version,"1.0",VERSION_LEN);
    memcpy(buffer,&head,sizeof(Proto_Head));
    std::string s_head(buffer,sizeof(Proto_Head));
    if(client.Send(s_head) != VERSION_LEN)
    {
        logger->info("send head failed\n");
    }
    client.Send(cmd);
}

std::string Recv_Msg(Client_Sock& client)
{
    std::string head_buffer;
    Proto_Head head;
    if(client.Recv(head_buffer,sizeof(Proto_Head)) == 0){
        logger->info("recv head failed\n");
    }
    memcpy(&head,head_buffer.c_str(),sizeof(Proto_Head));
    std::string buffer;
    client.Recv(buffer,head.length);
    return buffer;
}


int main(int argc,char* argv[])
{
    std::string path;
    if(argc < 2){
        path="./demo.json";
    }else if(argc == 2){
        path = argv[1];
    }else{
        printf("arg error!\n");
    }
    Config_NSP::Config_Json json;
    json.Load_Conf(path);
    json.Get_Value<std::string>(json.Get_Root_Value(),"remote_addr",remote_addr);
    json.Get_Value<int>(json.Get_Root_Value(),"remote_port",remote_port);
    // 创建静态日志器并添加文件旋转器作为输出目标

    Client_Sock client;
    if(client.Sock() <= 0)
    {
        logger->error("sock init failed\n");
        exit(-1);
    }
    if(client.Connect(remote_addr,remote_port))
    {
        logger->error("connect filed\n");
        exit(-1);
    }
    while(1)
    {
        std::string cmd ="";
        std::cout<<"kvstore-cli > ";
        std::getline(std::cin,cmd);
        //去除前缀空格
        for(int i=0;i<cmd.length();i++)
        {
            if(cmd[i]!=' ') break;
            else{
                cmd.erase(i,1);
                i--;
            }
        }
        for(int i=0;i<cmd.length();i++)
        {
            if(cmd[i] == ' ') break;
            cmd[i] = std::toupper(cmd[i]);
        }
        if(cmd.compare("QUIT") == 0) break;
        Send_Msg(client,cmd);
        std::cout<<Recv_Msg(client)<<std::endl;

    }
    client.Close();
    std::cout<<"Bye"<<std::endl;
    return 0;
}