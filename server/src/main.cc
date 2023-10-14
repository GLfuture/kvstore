/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-29 05:40:03
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-14 20:27:35
 */

#include "app/app.h"
#include "config/config.h"
#include <cerrno>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"


#define THREAD_NUM 8
#define MAX_CONN_NUM 10
#define PRORO_VERION "1.0"
#define VERION_LENGTH 4
#define LOGGER_NAME "logger"
#define MAX_LOG_SIZE 10*1024*1024
#define MAX_LOG_FILE_NUM 5
#define FORK_PROCESS_NUM 4
#define LOG_LEVEL spdlog::level::debug

using namespace Config_NSP;

static bool Is_Exist = false;
static int port = 8080;
static int backlog = 10;
static int event_num = 2048;
static int thread_num = 4;
static int lock_wait_millisecond = 100;
static std::string aof_path = "./kvstore.aof";
static std::string aof_back_path = "./aof.bk";
static std::string log_path = "../logs/kv.log";

/**
 * @description: 接收连接触发回调
 * @param {Reactor} *R
 * @param {Server_Ptr} server
 * @return void
 * @author: Gong
 */
void Accept_cb(Reactor *R)
{

    APP::Server_Ptr server = std::dynamic_pointer_cast<Server>(R->Get_Server());
    int clinetfd = server->Accept();
    APP::Tcp_Conn_Ptr conn = std::make_shared<Tcp_Conn>(clinetfd);
    server->Add_Conn(conn);
    R->Add_Reactor(clinetfd, EPOLLIN);
}


/**
 * @description: 读事件触发回调
 * @param {APP} *app            //主应用程序,包含业务层
 * @param {Reactor} *R          //Reactor，管理连接
 * @param {Server_Ptr} server   //server的指针
 * @param {ThreadPool} &th_pool //线程池,用于并发处理协议
 * @return void
 * @author: Gong
 */
void Read_cb(APP* app,Reactor *R, ThreadPool &th_pool)
{
    int clientfd = R->Get_Now_Event().data.fd;
    APP::Server_Ptr server = std::dynamic_pointer_cast<Server>(R->Get_Server());
    APP::Tcp_Conn_Ptr conn = std::dynamic_pointer_cast<Tcp_Conn>(server->Get_Conn(clientfd));
    int proto_len = 0;
    Proto_Head head;
    //头中携带len信息，防止Tcp粘包
    int len = server->Recv_Rroto_Head(clientfd,head);
    if (len <= 0)
    {
        app->Deal_Closed_Conn(conn,EPOLLIN);
        server->Close(clientfd);
        std::cout<<"close fd :"<<clientfd<<std::endl;
        return;
    }
    //std::cout<<head.length<<std::endl;
    len = server->Recv(conn,head.length);
    if(len != head.length){
        return;
    }
    if(len <= 0){
        app->Deal_Closed_Conn(conn,EPOLLIN);
        server->Close(clientfd);
        std::cout<<"close fd :"<<clientfd<<std::endl;
        return;
    }
    //std::cout<<"cmd: "<<conn->Get_Rbuffer()<<std::endl;
    std::future<string> res = th_pool.exec(std::bind(&APP::Work,app,conn,len));
    std::shared_future<string> share_res=res.share();
    app->Add_Future(conn,share_res);
}

/**
 * @description: 写事件触发回调
 * @param {APP} *app            //主应用程序,包含业务层
 * @param {Reactor} *R          //Reactor，管理连接
 * @param {Server_Ptr} server   //server的指针
 * @param {ThreadPool} &th_pool //线程池,用于并发处理协议
 * @return void
 * @author: Gong
 */


void Write_cb(Reactor *R)
{
    int clientfd = R->Get_Now_Event().data.fd;
    APP::Server_Ptr server = std::dynamic_pointer_cast<Server>(R->Get_Server());
    APP::Tcp_Conn_Ptr conn = std::dynamic_pointer_cast<Tcp_Conn>(server->Get_Conn(clientfd));
    string res;
    res = conn->Future_Get_An_Finished();
    //std::cout<<res<<std::endl;
    if(!res.empty()){
        if(!*conn->Get_Affairs_Status()){
            conn->Appand_Wbuffer(res);
            Proto_Head head;
            memcpy(head.version,PRORO_VERION,VERION_LENGTH);
            head.length = conn->Get_Wbuffer_Length();
            int len = server->Send_Proto_Head(clientfd,std::move(head));
            //std::cout<<"send head len : "<<len <<std::endl;
            len = server->Send(conn, conn->Get_Wbuffer_Length());
            //std::cout<<"send body len : "<<len<<std::endl;
            conn->Erase_Wbuffer(conn->Get_Wbuffer_Length());
        }
    }
    R->Mod_Reactor(clientfd,EPOLLIN);
}

int main(int argc,char*argv[])
{
    std::string path;
    if(argc < 2){
        path="./default.json";
        if(std::filesystem::exists(path)) {
            Is_Exist = true;
        }
    }else if(argc == 2){
        path = argv[1];
    }else{
        printf("arg error!\n");
    }
    auto m_logger = spdlog::rotating_logger_mt(LOGGER_NAME, log_path, MAX_LOG_SIZE, MAX_LOG_FILE_NUM);
    m_logger->set_level(LOG_LEVEL);
    spdlog::set_default_logger(m_logger);
    if(Is_Exist){
        Config_Json json;
        json.Load_Conf(path);
        int ret = json.Get_Value<int>(json.Get_Root_Value(), "port", port);
        if (ret == -1)
            exit(-1);
        ret = json.Get_Value<int>(json.Get_Root_Value(), "event_num", event_num);
        if (ret == -1)
            exit(-1);
        ret = json.Get_Value<int>(json.Get_Root_Value(), "backlog", backlog);
        if (ret == -1)
            exit(-1);
        ret = json.Get_Value<int>(json.Get_Root_Value(), "decode_thread_num", thread_num);
        if (ret == -1)
            exit(-1);
        ret = json.Get_Value<int>(json.Get_Root_Value(),"lock_wait_millisecond",lock_wait_millisecond);
        if(ret == -1)
            exit(-1);
        ret = json.Get_Value<std::string>(json.Get_Root_Value(),"aof_path",aof_path);
        if(ret == -1)
            exit(-1);
        ret = json.Get_Value<std::string>(json.Get_Root_Value(),"aof_back_path",aof_back_path);
        if(ret == -1)
            exit(-1);
        ret = json.Get_Value<std::string>(json.Get_Root_Value(),"log_path",log_path);
        if(ret == -1)
            exit(-1);
    }
    APP::Server_Ptr server = std::make_shared<Server>();
    if(server->Bind(port)!=0){
        spdlog::error("bind failed\n");
    }
    if(server->Listen(backlog)!=0)
    {
        spdlog::error("listen failed\n");
    }
    APP app(event_num, server, thread_num, lock_wait_millisecond, aof_path, aof_back_path);
    int ret = app.Read_AOF_And_Init();
    if (ret != 0)
    {
        std::cout << std::strerror(ret);
        exit(-1);
    }

    std::function<void()> accptcb = std::bind(Accept_cb,app.Get_Reactor());
    std::function<void()> readcb = std::bind(Read_cb, &app, app.Get_Reactor(), std::ref(app.Get_Thread_Pool()));
    std::function<void()> writecb = std::bind(Write_cb, app.Get_Reactor());
    std::function<void()> exitcb = NULL;
    // app.Init(atoi(argv[1]),MAX_CONN_NUM);
    app.Init_cb(accptcb, readcb, writecb, exitcb);
    app.Run();

    return 0;
}