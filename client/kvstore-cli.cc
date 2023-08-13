#include"Disposition/Disposition.hpp"
#include"Client_Socket/Client_Socket.hpp"
#include<string.h>
#define MAX_COMMAND_LENGTH 64
enum COMMAND{
    COMMAND_BEG = 0,
    COMMAND_QUIT = COMMAND_BEG,
    COMMAND_END,
};

const char*command[]={
    "quit"
};

CString Decode(CString buffer)
{
    return buffer.substr(0,buffer.length()-2);
}

int main(int argc,char* argv[])
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
    Client_Socket client(json._remote_addr,json._remote_port,json._buffer_size);
    int clientfd = client.Socket_Init();
    if(client.Connect()==-1){
        printf("connect failed!\n");
        exit(0);
    }
    char* buffer=new char[MAX_COMMAND_LENGTH];
    std::cout<<"kvstore-cli > ";
    std::cin.getline(buffer,MAX_COMMAND_LENGTH);
    while(strcmp(buffer,command[COMMAND_QUIT])!=0){
        string msg(buffer);
        client.Set_Write_Buffer(msg);
        client.Send(json._send_length);
        if(client.Recv(json._recv_length)==-1){
            printf("Disconnction\n");
            exit(-1);
        }
        std::cout<<Decode(client.Get_Read_Buffer())<<std::endl;
        client.Clean_Read_Buffer();
        client.Clean_Send_Buffer();
        std::cout<<"kvstore-cli > ";
        memset(buffer,0,MAX_COMMAND_LENGTH);
        std::cin.getline(buffer,MAX_COMMAND_LENGTH);
    }
    std::cout<<"Bye"<<std::endl;
    return 0;
}