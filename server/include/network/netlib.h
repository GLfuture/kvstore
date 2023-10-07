/*
 * @Description: 实现Conn和Server实例
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-30 13:56:13
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-07 08:13:16
 */
#pragma once
#include "reactor.h"
#include "../protocol/protocol.hpp"
#include "../security/security.h"
#include <queue>
#include <future>
#include <mutex>
#include <atomic>
#define SLEEP_MILLISEC_TIME 10



class Tcp_Conn:public Tcp_Conn_Base
{
public:
    Tcp_Conn(uint32_t conn_fd):Tcp_Conn_Base(conn_fd){
        results = std::make_shared<Security_Future_Queue>();
        Affairs_Start = std::make_shared<std::atomic_bool>(false);
        // Exec_Event.store(false);
    }
    ~Tcp_Conn() override{
        
    }

    void Future_Add(std::shared_future<string>& res_ptr)
    {
        std::lock_guard lock(*(results->mtx));
        results->seurity_queue.push(res_ptr);
    }

    // void Ready_Add_Cmd(std::string &cmd){
    //     std::lock_guard lock(*(ready_cmds->mtx));
    //     ready_cmds->seurity_queue.push(cmd);
    // }

    
    // std::string Ready_Get_Cmd(){
    //     std::lock_guard lock(*(ready_cmds->mtx));
    //     std::string cmd = ready_cmds->seurity_queue.front();
    //     ready_cmds->seurity_queue.pop();
        
    //     return cmd;
    // }

    // size_t Ready_Cmds_Size(){ return ready_cmds->seurity_queue.size(); }

    // bool Ready_Cmds_Empty(){ return ready_cmds->seurity_queue.empty();}

    // void Compile_Cmds_Clear_Up()
    // {
    //     compile_cmds->security_vector.clear();
    // }

    // void Compile_Add_Cmd(std::string& cmd)
    // {
    //     compile_cmds->security_vector.push_back(cmd);
    // }

    // std::shared_ptr<Security_Cmd_Vector>  Compile_Cmds_Get()
    // {
    //     return this->compile_cmds;
    // }

    // void Ready_Cmds_Clear_Up()
    // {
    //     while (!ready_cmds->seurity_queue.empty())
    //     {
    //         ready_cmds->seurity_queue.pop();
    //     }
        
    // }

    bool Future_Has_Finished()
    {
        // if (results->seurity_queue.front().wait_for(std::chrono::seconds(0)) == std::future_status::deferred)
        //     std::cout << "deferred" << std::endl;
        // if(results->seurity_queue.front().wait_for(std::chrono::seconds(0)) == std::future_status::timeout)
        //     std::cout<<"timeout"<<std::endl;
        if(!results->seurity_queue.empty() && results->seurity_queue.front().valid()){
            if(results->seurity_queue.front().wait_for(std::chrono::seconds(0)) == std::future_status::ready){
                return true;
            }
        }
        return false;
    }

    std::string Future_Get_An_Finished()
    {
        std::lock_guard lock(*(results->mtx));
        std::string ret = std::move(results->seurity_queue.front().get());
        results->seurity_queue.pop();
        return ret;
    }


    bool Future_Is_Empty(){ return results->seurity_queue.empty();  }

    // bool Get_Event_Start_Status(){ return Event_Start.load(); }

    // void Set_Event_Start_Status(bool Is_Start){ Event_Start.store(Is_Start); }

    // bool Get_Event_Exec_Status(){ return Exec_Event.load(); }

    // void Set_Event_Exec_Status(bool Is_Exec){ Exec_Event.store(Is_Exec);}
    std::shared_ptr<std::atomic_bool> Get_Affairs_Status(){
        return Affairs_Start;
    }

private:
    // //事务回滚集
    // std::shared_ptr<Security_Cmd_Vector> compile_cmds;
    // //事务命令集
    // std::shared_ptr<Security_Cmd_Queue> ready_cmds;
    std::shared_ptr<std::atomic_bool> Affairs_Start;
    //异步结果集
    std::shared_ptr<Security_Future_Queue> results;
};

class Server:public Server_Base
{
public:
    using Tcp_Conn_Ptr = std::shared_ptr<Tcp_Conn>;
    int Recv_Rroto_Head(int fd,Proto_Head& head)
    {
        char *buffer = new char[sizeof(Proto_Head)];
        memset(buffer,0,sizeof(Proto_Head));
        int len = recv(fd,buffer,sizeof(Proto_Head),0);
        memcpy(&head,buffer,sizeof(Proto_Head));
        return len;
    }

    void Finish_All_Future_Before_Del_Conn(Tcp_Conn_Ptr& conn_ptr) 
    {
        Finish_All_Future(conn_ptr);
        Close(conn_ptr->Get_Conn_fd());
        Del_Conn(conn_ptr->Get_Conn_fd());
    }

    ~Server() override {

    }

private:
    void Finish_All_Future(Tcp_Conn_Ptr& conn_ptr)
    {
        while(!conn_ptr->Future_Is_Empty()){
            if(conn_ptr->Future_Has_Finished()){
                conn_ptr->Future_Get_An_Finished();
            }else{
                std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MILLISEC_TIME));
            }
        }
    }
};