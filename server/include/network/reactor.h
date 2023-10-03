/*
 * @Description: 底层网络接口
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-30 13:24:15
 * @LastEditors: Gong
 * @LastEditTime: 2023-09-30 13:52:14
 */
#pragma once
#include <cstdint>
#define ENABLE_RBTREE_TIMER 1
#include "server.h"
#include "timermanager.hpp"
#include <sys/epoll.h>
#include <fcntl.h>
#include <functional>

using std::function;
// 注意一个端口对应一个reactor管理
class Reactor
{
public:
    using Server_Base_Ptr = shared_ptr<Server_Base>;
    using Timer_Ptr = std::shared_ptr<Timer>;
    /// @brief
    /// @param event_num
    /// @param buffer_size
    Reactor(uint16_t event_num);
    // 返回server对象
    Server_Base_Ptr Get_Server();

    uint16_t Add_Reactor(int fd, uint32_t event);

    uint16_t Del_Reactor(int fd, uint32_t event);
    

    uint16_t Mod_Reactor(int fd, uint32_t event);

    // 设置非阻塞
    void Set_No_Block(int fd);

    // 设置阻塞
    void Set_Block(int fd);

    void Add_Server(Server_Base_Ptr server);

    void Exit();

    // 获取事件数量
    uint32_t Get_Event_Num() { return this->event_num; }

    // 返回当前事件
    epoll_event Get_Now_Event() { return *event; }

    // 事件主循环,默认死等
    void Event_Loop(uint16_t timeout = -1);

    void Set_Accept_cb(function<void()> &&accept_cb) { this->Accept_cb = accept_cb; }

    void Del_Accept_cb() { this->Accept_cb = NULL; }

    void Set_Read_cb(function<void()> &&read_cb) { this->Read_cb = read_cb; }

    void Del_Read_cb() { this->Read_cb = NULL; }

    void Set_Write_cb(function<void()> &&write_cb) { this->Write_cb = write_cb; }

    void Del_Write_cb() { this->Write_cb = NULL; }

    void Set_Exit_cb(function<void()> &&exit_cb) { this->exit_cb = exit_cb; }

    void Del_Exit_cb() { this->exit_cb = NULL; }

#if ENABLE_RBTREE_TIMER | ENABLE_MINHEAP_TIMER
    Timer_Ptr Set_Timeout_cb(uint16_t timerid, uint64_t interval_time, Timer::TimerType type, function<void()> &&timeout_cb)
    {
        return timermanager.Add_Timer(timerid, interval_time, type, timeout_cb);
    }
#elif ENABLE_TIMERWHEEL_TIMER
    Timer_Ptr Set_Timeout_cb(uint16_t timerid, uint64_t interval_time, function<void()> &&timeout_cb)
    {
        return timermanager.Add_Timer(timerid, timeout_cb, interval_time);
    }
#endif
#if ENABLE_RBTREE_TIMER
    void Del_Timeout_cb(uint16_t timerid) { timermanager.Del_Timer(timerid); }
#endif
    void Del_Timeout_cb(Timer_Ptr &timer) { timermanager.Del_Timer(timer); }

private:
    function<void()> Accept_cb;
    function<void()> Read_cb;
    function<void()> Write_cb;
    function<void()> exit_cb;

private:
    uint16_t epfd;
    Server_Base_Ptr _server;
    bool quit;
    uint32_t event_num;
    epoll_event *event;
    // 当前event
    TimerManager timermanager;
};
