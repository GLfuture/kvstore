/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-29 05:40:03
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-03 08:33:21
 */

#include"app/app.h"
#include "config/config.h"

#define THREAD_NUM 8
#define MAX_CONN_NUM 10
using namespace Config_NSP;

static bool Is_Exist = false;

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
    APP app(port,backlog,thread_num,event_num,per_max_buffer_size,100); 
    //app.Init(atoi(argv[1]),MAX_CONN_NUM);
    app.Run();
    return 0;
}