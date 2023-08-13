#pragma once
#include <cstdint>
#define ENABLE_RBTREE_TIMER 1
#include "Server_Socket/Server_Socket.hpp"
#include "timer/timermanager.hpp"
#include <sys/epoll.h>
#include <fcntl.h>
#include <functional>
namespace Reactor_NSP
{
    using Timer_Ptr = std::shared_ptr<Timer>;
    using std::function;
    using namespace Server_Socket_NSP;
    using Server_Ptr = shared_ptr<Server_Socket>;

    // 注意一个端口对应一个reactor管理
    class Reactor
    {
    public:
        Reactor(){
            Reactor(1024,1024);
        }
        Reactor(uint16_t event_num,uint32_t buffersize)
        {
            this->epfd = epoll_create(1);
            this->quit = false;
            this->Accept_cb = NULL;
            this->Read_cb = NULL;
            this->Write_cb = NULL;
            this->event_num = event_num;
            this->event = new epoll_event;
            Server_Socket *S = new Server_Socket(buffersize);
            this->server = std::make_shared<Server_Socket>(S);
            int timefd = timermanager.Create_Timerfd();
            this->Add_Reactor(timefd, EPOLLIN | EPOLLET);
        }
        // 返回server对象
        Server_Ptr Get_Server()
        {
            return this->server;
        }

        uint16_t Add_Reactor(int fd, uint32_t event)
        {
            epoll_event ev = {0};
            ev.events = event;
            ev.data.fd = fd;
            uint16_t ret = epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &ev);
            return ret;
        }

        uint16_t Del_Reactor(int fd, uint32_t event)
        {
            epoll_event ev = {0};
            ev.events = event;
            ev.data.fd = fd;
            uint16_t ret = epoll_ctl(this->epfd, EPOLL_CTL_DEL, fd, &ev);
            return ret;
        }

        uint16_t Mod_Reactor(int fd, uint32_t event)
        {
            epoll_event ev = {0};
            ev.events = event;
            ev.data.fd = fd;
            uint16_t ret = epoll_ctl(this->epfd, EPOLL_CTL_MOD, fd, &ev);
            return ret;
        }

        // 设置非阻塞
        void Set_No_Block(int fd)
        {
            int flag = fcntl(fd, F_GETFL, 0);
            flag |= O_NONBLOCK;
            fcntl(fd, F_SETFL, flag);
        }
        // 设置阻塞
        void Set_Block(int fd)
        {
            int flag = fcntl(fd, F_GETFL, 0);
            flag &= ~O_NONBLOCK;
            fcntl(fd, F_SETFL, flag);
        }

        void Exit()
        {
            this->quit = true;
        }

        // 获取事件数量
        uint32_t Get_Event_Num()
        {
            return this->event_num;
        }

        // 返回当前事件
        epoll_event Get_Now_Event()
        {
            return *event;
        }

        // 事件主循环,默认死等
        void Event_Loop(uint16_t timeout = -1)
        {
            epoll_event *events = new epoll_event[this->event_num];
            while (!this->quit)
            {
                timermanager.Update_Timerfd();
                int ready = epoll_wait(this->epfd, events, this->event_num, timeout); // 默认死等
                if (ready == 0)
                    continue;
                for (int i = 0; i < ready; i++)
                {
                    *event = events[i];
                    if (events[i].data.fd == server->Get_Sock())
                    {
                        if (this->Accept_cb)
                            this->Accept_cb();
                    }
                    else if (events[i].events == EPOLLIN)
                    {
                        if (timermanager.Get_Timerfd() == events[i].data.fd)
                            timermanager.Tick();
                        else
                        {
                            if (this->Read_cb)
                                this->Read_cb();
                        }
                    }
                    else if (events[i].events == EPOLLOUT)
                    {
                        if (this->Write_cb)
                            this->Write_cb();
                    }
                }
            }
        }

        void Set_Accept_cb(function<void()> &&accept_cb)
        {
            this->Accept_cb = accept_cb;
        }

        void Del_Accept_cb()
        {
            this->Accept_cb = NULL;
        }

        void Set_Read_cb(function<void()> &&read_cb)
        {
            this->Read_cb = read_cb;
        }

        void Del_Read_cb()
        {
            this->Read_cb = NULL;
        }

        void Set_Write_cb(function<void()> &&write_cb)
        {
            this->Write_cb = write_cb;
        }

        void Del_Write_cb()
        {
            this->Write_cb = NULL;
        }

#if ENABLE_RBTREE_TIMER|ENABLE_MINHEAP_TIMER
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
        void Del_Timeout_cb(uint16_t timerid)
        {
            timermanager.Del_Timer(timerid);
        }
#endif
        void Del_Timeout_cb(Timer_Ptr &timer)
        {
            timermanager.Del_Timer(timer);
        }

    private:
        function<void()> Accept_cb;
        function<void()> Read_cb;
        function<void()> Write_cb;

    private:
        uint16_t epfd;
        Server_Ptr server;
        bool quit;
        uint32_t event_num;
        epoll_event *event;
        // 当前event
        TimerManager timermanager;
    };
}