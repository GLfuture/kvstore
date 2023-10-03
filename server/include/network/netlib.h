/*
 * @Description: 实现Conn和Server实例
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-30 13:56:13
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-03 13:33:52
 */
#pragma once
#include "reactor.h"
#include <queue>
#include <future>
#include <mutex>
class Security_Future_Queue
{
public:
    Security_Future_Queue(){
        mtx = std::make_shared<std::mutex>();
    }
    Security_Future_Queue(const Security_Future_Queue&& other){
        this->mtx = std::move(other.mtx);
        this->seurity_queue = std::move(other.seurity_queue);
    }
    Security_Future_Queue(const Security_Future_Queue*& other){
        this->mtx = std::move(other->mtx);
        this->seurity_queue = std::move(other->seurity_queue);
    }
    std::shared_ptr<std::mutex> mtx;
    std::queue<std::shared_future<std::string>> seurity_queue;
};


class Tcp_Conn:public Tcp_Conn_Base
{
public:
    Tcp_Conn(uint32_t conn_fd):Tcp_Conn_Base(conn_fd){
        results = std::make_shared<Security_Future_Queue>();
    }
    ~Tcp_Conn() override{
        
    }

    void Add_Future(std::shared_future<string>& res_ptr)
    {
        std::lock_guard lock(*(results->mtx));
        results->seurity_queue.push(res_ptr);
    }

    bool Has_Finished_Future()
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

    std::string Get_An_Finish_Task()
    {
        std::lock_guard lock(*(results->mtx));
        std::string ret = std::move(results->seurity_queue.front().get());
        results->seurity_queue.pop();
        return ret;
    }


private:
    std::shared_ptr<Security_Future_Queue> results;
};

class Server:public Server_Base
{
public:
    template<class T>
    ssize_t Recv_Len(const Tcp_Conn_Base_Ptr&& conn_ptr,T& len)
    {
        return  recv(conn_ptr->Get_Conn_fd(),&len,sizeof(len),0);
    }

    ~Server() override {

    }
};