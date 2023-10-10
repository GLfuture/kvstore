/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-29 05:40:03
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-10 08:41:03
 */

#include"app/app.h"
#include "config/config.h"
#include <cerrno>
#define THREAD_NUM 8
#define MAX_CONN_NUM 10
#define PRORO_VERION "1.0"
#define VERION_LENGTH 4
using namespace Config_NSP;

static bool Is_Exist = false;


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
        return;
    }
    len = server->Recv(conn,head.length);
    if(len <= 0){
        R->Del_Reactor(clientfd,EPOLLIN);
        server->Close(clientfd);
        return;
    }
    //std::cout<<"cmd: "<<conn->Get_Rbuffer()<<std::endl;
    std::future<string> res = th_pool.exec(std::bind(&APP::Work,app,conn));
    std::shared_future<string> share_res=res.share();
    conn->Future_Add(share_res);
    R->Mod_Reactor(clientfd,EPOLLOUT);
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
    while(!conn->Future_Has_Finished()){
    };
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
    int port = 8080;
    int backlog = 10;
    int event_num = 2048;
    int thread_num = 4;
    int per_max_buffer_size = 1024;
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
        ret = json.Get_Value<int>(json.Get_Root_Value(), "thread_num", thread_num);
        if (ret == -1)
            exit(-1);
        ret = json.Get_Value<int>(json.Get_Root_Value(), "per_max_buffer_size", per_max_buffer_size);
        if (ret == -1)
            exit(-1);
    }
    
    APP app(event_num,port,backlog,thread_num,100,"test.txt","aof.bk");
    int ret = app.Read_AOF_And_Init();
    if(ret != 0) 
    {
        std::cout << std::strerror(ret);
        exit(-1);
    }
    std::function<void()> accptcb = std::bind(Accept_cb,app.Get_Reactor());
    std::function<void()> readcb = std::bind(Read_cb,&app,app.Get_Reactor(),std::ref(app.Get_Thread_Pool()));
    std::function<void()> writecb = std::bind(Write_cb,app.Get_Reactor()); 
    std::function<void()> exitcb = NULL;
    //app.Init(atoi(argv[1]),MAX_CONN_NUM);
    app.Init_cb(accptcb,readcb,writecb,exitcb);
    app.Run();
    return 0;
}