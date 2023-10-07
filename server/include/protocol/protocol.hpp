/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-10-05 07:59:08
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-07 11:17:07
 */
#pragma once
#include <iostream>
#include <string.h>
#include "../security/security.h"
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

class Msg
{
public:
    Msg(int fd , std::string buffer, std::shared_ptr<std::atomic_bool> status)
    {
        this->fd = fd;
        this->buffer = buffer;
        Event_Start = status;
    }
    int fd;
    std::shared_ptr<std::atomic_bool> Event_Start;
    std::string buffer;
};

class Affairs
{
public:
    Affairs()
    {
        ready_cmds = std::make_shared<Security_Cmd_Queue>();
    }

    Affairs(Affairs*& other){
        this->ready_cmds = std::move(other->ready_cmds);
    }
    void Ready_Add_Cmd(std::string cmd)
    {
        ready_cmds->seurity_queue.push(cmd);
    }

    std::string Get_An_Cmd()
    {
        std::string res = ready_cmds->seurity_queue.front();
        ready_cmds->seurity_queue.pop();
        return res;
    }

    std::shared_ptr<Security_Cmd_Queue> ready_cmds;
};