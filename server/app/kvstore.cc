
#include"app.h"
#include "../Disposition/Disposition.hpp"

#define THREAD_NUM 8
#define MAX_CONN_NUM 10

int main(int argc,char*argv[])
{
    std::string path;
    if(argc < 2){
        path="./demo.json";
    }else if(argc == 2){
        path = argv[1];
    }else{
        printf("arg error!\n");
    }
    Disposition json(path.c_str());
    APP app(json._port,json._backlog,json._thread_num,json._event_num,json._buffer_size); 
    //app.Init(atoi(argv[1]),MAX_CONN_NUM);
    app.Run();
    return 0;
}