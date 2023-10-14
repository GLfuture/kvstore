/*
 * @Description: 实现Conn和Server实例
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-30 13:56:13
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-14 17:22:08
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

class Server;


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
        std::unique_lock lock(*(results->mtx));
        Security_Future res;
        res.Set_Shared_Future(res_ptr);
        results->security_queue.push(res);
        
    }

    // void Ready_Add_Cmd(std::string &cmd){
    //     std::lock_guard lock(*(ready_cmds->mtx));
    //     ready_cmds->security_queue.push(cmd);
    // }

    
    // std::string Ready_Get_Cmd(){
    //     std::lock_guard lock(*(ready_cmds->mtx));
    //     std::string cmd = ready_cmds->security_queue.front();
    //     ready_cmds->security_queue.pop();
        
    //     return cmd;
    // }

    // size_t Ready_Cmds_Size(){ return ready_cmds->security_queue.size(); }

    // bool Ready_Cmds_Empty(){ return ready_cmds->security_queue.empty();}

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
    //     while (!ready_cmds->security_queue.empty())
    //     {
    //         ready_cmds->security_queue.pop();
    //     }
        
    // }

    bool Future_Has_Finished()
    {
        if(!results->security_queue.front().Is_Finished.load()&&!results->security_queue.empty() && results->security_queue.front().res.valid()){
            //这个地方会存在pop问题，导致front为空，发生段错误
            if(results->security_queue.front().res.wait_for(std::chrono::seconds(0)) == std::future_status::ready){

                return true;
            }
        }
        return false;
    }

    std::string Future_Get_An_Finished()
    {
        std::unique_lock lock(*(results->mtx));
        std::string ret = std::move(results->security_queue.front().res.get());
        results->security_queue.pop();
        return ret;
    }


    bool Future_Is_Empty(){ return results->security_queue.empty();  }

    // bool Get_Event_Start_Status(){ return Event_Start.load(); }

    // void Set_Event_Start_Status(bool Is_Start){ Event_Start.store(Is_Start); }

    // bool Get_Event_Exec_Status(){ return Exec_Event.load(); }

    // void Set_Event_Exec_Status(bool Is_Exec){ Exec_Event.store(Is_Exec);}
    std::shared_ptr<std::atomic_bool> Get_Affairs_Status(){
        return Affairs_Start;
    }

private:
    friend class Server;
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

    int Send_Proto_Head(int fd,const Proto_Head&& head)
    {
        char * buffer = new char[sizeof(Proto_Head)];
        memset(buffer, 0 ,sizeof(Proto_Head));
        memcpy(buffer,&head,sizeof(Proto_Head));
        return send(fd,buffer,sizeof(Proto_Head),0);
    }

    void Finish_All_Future_Before_Del_Conn(Tcp_Conn_Ptr& conn_ptr) 
    {
        Finish_All_Future(conn_ptr);
        Close(conn_ptr->Get_Conn_fd());
        Del_Conn(conn_ptr->Get_Conn_fd());
    }

    void Check_Conns_Has_Fininshed_Task(Reactor* R)
    {
        for(map<uint32_t,Tcp_Conn_Base_Ptr>::iterator it = connections.begin();it!=connections.end();it++)
        {
            std::shared_lock lock(*std::dynamic_pointer_cast<Tcp_Conn>(it->second)->results->mtx);
            if(std::dynamic_pointer_cast<Tcp_Conn>(it->second)->Future_Has_Finished()){
                lock.unlock();
                R->Mod_Reactor(it->first,EPOLLOUT);
            }
        }
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