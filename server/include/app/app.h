#pragma once
#include <cstdint>
#ifndef APP_H
#define APP_H 
#include "../network/netlib.h"
#include "../thread_pool/ThreadPool.hpp"
#include "../split/split.hpp"
#include "../spdlog/spdlog.h"
#include "security.h"
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
    "END"
};

typedef enum RETCODE
{
    RET_BEG,
    RET_OK = RET_BEG,
    RET_EXIST,
    RET_NOT_EXIST,
    RET_NO_KEY,
    RET_NO_FIELD,
    RET_STR_ERROR,
    RET_ARG_ERROR,
    RET_CMD_ERROR,
    RET_END,
} RETCODE;

static const char *ret_msg[RET_END]=
{
    "OK\r\n",
    "EXIST\r\n",
    "NOT EXIST\r\n",
    "NO KEY\r\n",
    "NO FIELD\r\n",
    "ERROR_STR\r\n",
    "ERROR_ARG\r\n",
    "ERROR_CMD\r\n",
};

#endif

class APP
{
public:
    using Server_Ptr = std::shared_ptr<Server>;
    using Tcp_Conn_Ptr = std::shared_ptr<Tcp_Conn>;
    APP(uint16_t port,uint16_t backlog,uint8_t thread_num,uint32_t event_num,uint32_t buffer_size,int64_t wait_for_lock_millisec)
    {
        wait_lock_millisec = wait_for_lock_millisec;
        R = new Reactor(event_num);
        Init(port,backlog,thread_num,buffer_size);
    }
    int Init(uint16_t port,uint16_t backlog,uint8_t thread_num,uint32_t per_max_rcv_size);

    string Decode(string buffer);

    string Work(APP::Tcp_Conn_Ptr& conn);
    
    void Run(){
        R->Event_Loop();
    }
private:


    string Decode_String(int cmd_type,vector<string_view>& res);
    string Decode_Array(int cmd_type,vector<string_view>& res);
    string Decode_List(int cmd_type,vector<string_view>& res);
    string Decode_Rbtree(int cmd_type,vector<string_view>& res);
    string Decode_Set(int cmd_type,vector<string_view>& res);
    //string Decode_Skiptable(int cmd_type,vector<string_view>& res);

    //--------------------- 字符串 -------------------------
    string Exec_Cmd_Set(string key,string value);
    string Exec_Cmd_Get(string key);
    string Exec_Cmd_Appand(string key,string value);
    string Exec_Cmd_Len(string key);
    string Exec_Cmd_Delete(string key);
    string Exec_Cmd_Exist(string key);

    //--------------------- 数组 -------------------------
    string Exec_Cmd_ASet(string key,vector<string_view> &res);
    string Exec_Cmd_AGet(string key);
    string Exec_Cmd_ACount(string key);
    string Exec_Cmd_ADelete(string key,vector<string_view> &res);
    string Exec_Cmd_AExist(string key,string value);

    //--------------------- 链表 -------------------------
    string Exec_Cmd_LPUSH(string key,vector<string_view> &res);
    string Exec_Cmd_RPUSH(string key,vector<string_view> &res);
    string Exec_Cmd_LGet(string key);
    string Exec_Cmd_LCount(string key);
    string Exec_Cmd_LDelete(string key,vector<string_view> &res);
    string Exec_Cmd_LExist(string key,string value);


    //--------------------- 红黑树 -------------------------
    string Exec_Cmd_RSet(string key,vector<string_view> &res);
    string Exec_Cmd_RGet(string key,string field);
    string Exec_Cmd_RCount(string key);
    string Exec_Cmd_RDelete(string key,vector<string_view> &res);
    string Exec_Cmd_RExist(string key,string field);
    
    //--------------------- 集合 -------------------------
    string Exec_Cmd_SSet(string key,vector<string_view> &res);
    string Exec_Cmd_SGet(string key);
    string Exec_Cmd_SCount(string key);
    string Exec_Cmd_SDelete(string key,vector<string_view> &res);
    string Exec_Cmd_SExist(string key,string value);


private:

    ThreadPool th_pool; // 线程池
    Reactor *R;       
    int64_t wait_lock_millisec;

    Map_Security_String_Store string_store;//string存储
    
    Map_Security_Array_Store array_store;//array存储

    Map_Security_List_Store list_store;//list存储

    Map_Security_RBtree_Store rbtree_store;//rbtree存储

    Map_Security_Set_Store set_store;//set存储
};
