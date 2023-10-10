#pragma once
#include <cstdint>
#ifndef APP_H
#define APP_H 
#include "network/netlib.h"
#include "thread_pool/ThreadPool.hpp"
#include "split/split.hpp"
#include "protocol/protocol.hpp"
#include "security/security.h"
#include "fileio/fileio.h"
#include "spdlog/spdlog.h"
#include <queue>
#include <unordered_set>
#include <list>
#include <span>
using std::map;
using std::queue;
using std::span;
using std::unordered_set;
using std::list;
#define THREAD_NUM 8

#define LIMIT_STREAM_SIZE 10*1024*1024
//后期可将一部分string优化为string_view，减少拷贝
typedef enum CMD{
    CMD_BEG = 0,
    CMD_SET = CMD_BEG,
    CMD_GET,
    CMD_APPAND,
    CMD_LEN,
    CMD_DELETE,
    CMD_EXIST,
    CMD_ASET,
    CMD_AGET,
    CMD_ACOUNT,
    CMD_ADELETE,
    CMD_AEXIST,
    CMD_LPUSH,
    CMD_RPUSH,
    CMD_LGET,
    CMD_LCOUNT,
    CMD_LDELETE,
    CMD_LEXIST,
    CMD_RSET,
    CMD_RGET,
    CMD_RCOUNT,
    CMD_RDELETE,
    CMD_REXIST,
    CMD_SSET,
    CMD_SGET,
    CMD_SCOUNT,
    CMD_SDELETE,
    CMD_SEXIST,
    CMD_EVENTBEG,
    CMD_EVENTEND,
    CMD_ROLLBACK,
    CMD_CLEAN_CACHE,
    CMD_CLEAN_AOF,
    CMD_END,
} CMD;

static const char *cmds[CMD_END]={
    "SET",
    "GET",
    "APPAND",
    "LEN",
    "DELETE",
    "EXIST",
    "ASET",
    "AGET",
    "ACOUNT",
    "ADELETE",
    "AEXIST",
    "LPUSH",
    "RPUSH",
    "LGET",
    "LCOUNT",
    "LDELETE",
    "LEXIST",
    "RSET",
    "RGET",
    "RCOUNT",
    "RDELETE",
    "REXIST",
    "SSET",
    "SGET",
    "SCOUNT",
    "SDELETE",
    "SEXIST",
    "BEG",
    "END",
    "ROLLBACK",
    "CLEAN_CACHE",
    "CLEAN_AOF",
};

typedef enum RETCODE
{
    RET_BEG,
    RET_OK = RET_BEG,
    RET_EXIST,
    RET_NOT_EXIST,
    RET_NO_KEY,
    RET_NO_FIELD,
    RET_NULL,
    RET_STR_ERROR,
    RET_ARG_ERROR,
    RET_CMD_ERROR,
    RET_ROLLBACK_ERROR,
    RET_END,
} RETCODE;

static const char *ret_msg[RET_END]=
{
    "OK",
    "EXIST",
    "NOT EXIST",
    "NO KEY",
    "NO FIELD",
    "",
    "ERROR_STR",
    "ERROR_ARG",
    "ERROR_CMD",
    "ERROR_ROLLBACK_FILED"
};

#endif

class APP
{
public:
    using Server_Ptr = std::shared_ptr<Server>;
    using Tcp_Conn_Ptr = std::shared_ptr<Tcp_Conn>;
    using Affair_Ptr = std::shared_ptr<Affairs>;

    using File_IO_Ptr = std::shared_ptr<File_IO>;

    /**
     * @description: 
     * @param {uint32_t} event_num              //同时处理的事务的最大数量
     * @param {uint16_t} port                   //监听端口
     * @param {uint16_t} backlog                //半连接队列长度
     * @param {uint8_t} thread_num              //线程数
     * @param {uint64_t} wait_for_lock_millisec //等待锁时的休眠时间
     * @param {string} file_path                //aof文件保存路径
     * @param {string} bk_file_path             //上一次aof的备份
     * @return {*}
     * @author: Gong
     */
    APP(uint32_t event_num,uint16_t port, uint16_t backlog, uint8_t thread_num,uint64_t wait_for_lock_millisec
        ,std::string file_path,std::string bk_file_path)
    {
        file_io = std::make_shared<File_IO>(file_path,bk_file_path);
        wait_lock_millisec = wait_for_lock_millisec;
        R = new Reactor(event_num);
        th_pool.Create(thread_num);
        Server_Ptr server = std::make_shared<Server>();
        R->Add_Server(server);
        server->Bind(port);
        server->Listen(backlog);
        R->Set_No_Block(server->Get_Sock());
        R->Add_Reactor(server->Get_Sock(), EPOLLIN);
    }

    int Read_AOF_And_Init();

    int Init_cb(std::function<void()>&accept_cb,std::function<void()>&read_cb
        ,std::function<void()>&write_cb,std::function<void()>&exit_cb);

    Reactor* Get_Reactor(){ return this->R;}
    
    ThreadPool& Get_Thread_Pool(){ return this->th_pool;}

    void Run(){
        R->Event_Loop();
    }
    /**
     * @description: 处理连接断开后该连接未执行完的任务
     * @param {Tcp_Conn_Ptr&} conn_ptr 
     * @param {uint32_t} event //需要移除的事件类型
     * @return {*}
     * @author: Gong
     */    
    void Deal_Closed_Conn(Tcp_Conn_Ptr& conn_ptr,uint32_t event);

    Reactor::Timer_Ptr Add_Time_Out_cb(uint16_t timerid, uint64_t interval_time, Timer::TimerType type,function<void()> &&timeout_cb);

    string Work(APP::Tcp_Conn_Ptr& conn);
private:


    string Decode(Msg& msg);

    string Decode_String(Msg& msg, int cmd_type,vector<string_view>& res);
    string Decode_Array(Msg& msg,int cmd_type,vector<string_view>& res);
    string Decode_List(Msg& msg,int cmd_type,vector<string_view>& res);
    string Decode_Rbtree(Msg& msg,int cmd_type,vector<string_view>& res);
    string Decode_Set(Msg& msg,int cmd_type,vector<string_view>& res);
    //string Decode_Skiptable(int cmd_type,vector<string_view>& res);

    //--------------------- 字符串 -------------------------
    string Exec_Cmd_Set(Msg& msg, string key,string value);
    string Exec_Cmd_Get(string key);
    string Exec_Cmd_Appand(Msg& msg , string key,string value);
    string Exec_Cmd_Len(string key);
    string Exec_Cmd_Delete(Msg& msg, string key);
    string Exec_Cmd_Exist(string key);

    //--------------------- 数组 -------------------------
    string Exec_Cmd_ASet(Msg& msg,string key,vector<string_view> &res);
    string Exec_Cmd_AGet(string key);
    string Exec_Cmd_ACount(string key);
    string Exec_Cmd_ADelete(Msg& msg,string key,vector<string_view> &res);
    string Exec_Cmd_AExist(string key,string value);

    //--------------------- 链表 -------------------------
    string Exec_Cmd_LPUSH(Msg& msg,string key,vector<string_view> &res);
    string Exec_Cmd_RPUSH(Msg& msg,string key,vector<string_view> &res);
    string Exec_Cmd_LGet(string key);
    string Exec_Cmd_LCount(string key);
    string Exec_Cmd_LDelete(Msg& msg,string key,vector<string_view> &res);
    string Exec_Cmd_LExist(string key,string value);


    //--------------------- 红黑树 -------------------------
    string Exec_Cmd_RSet(Msg& msg,string key,vector<string_view> &res);
    string Exec_Cmd_RGet(string key,string field);
    string Exec_Cmd_RCount(string key);
    string Exec_Cmd_RDelete(Msg& msg,string key,vector<string_view> &res);
    string Exec_Cmd_RExist(string key,string field);
    
    //--------------------- 集合 -------------------------
    string Exec_Cmd_SSet(Msg& msg,string key,vector<string_view> &res);
    string Exec_Cmd_SGet(string key);
    string Exec_Cmd_SCount(string key);
    string Exec_Cmd_SDelete(Msg& msg,string key,vector<string_view> &res);
    string Exec_Cmd_SExist(string key,string value);

    //---------------------event---------------------
    string Exec_Cmd_Eevent_Beg(Msg& msg);
    string Exec_Cmd_End(Msg& msg);
    string Exec_Cmd_RollBack(Msg& msg);
    string Exec_Cmd_Clean_Cache(Msg& msg);
    string Exec_Cmd_Clean_AOF(Msg& msg);

    void BackUp();

private:

    ThreadPool th_pool; // 线程池
    Reactor *R;

    File_IO_Ptr file_io;


    uint64_t cmd_save_millisec;//定时保存命令到磁盘以防宕机或连接断开

    uint64_t wait_lock_millisec;

    Map_Security_String_Store string_store;//string存储
    
    Map_Security_Array_Store array_store;//array存储

    Map_Security_List_Store list_store;//list存储

    Map_Security_RBtree_Store rbtree_store;//rbtree存储

    Map_Security_Set_Store set_store;//set存储

    Security_Back_Store back_store;//存储DELETE key的信息

    std::map<uint32_t,Affair_Ptr> affairs;//存储各个连接对应的事务,每个连接同时只能执行一个事务
};
