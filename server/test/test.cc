/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-29 05:40:03
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-08 02:56:49
 */
#include<iostream>
#include<netinet/in.h>
#include<string.h>
#include"Client_Socket/Client_Socket.hpp"
using namespace Client_Socket_NSP;
#define VERSION_LEN 4
#define SERVER_IP "192.168.124.140"
class Proto_Head
{
public:
    Proto_Head(){
        memset(version,0,VERSION_LEN);
    }
    char version[VERSION_LEN];
    uint32_t length;
};

void  Decode_Proto_Head(string buffer,Proto_Head& head)
{
    memcpy(&head,buffer.c_str(),sizeof(Proto_Head));

}


void Test(Client_Socket& client ,string send_msg,string except,const char* func, bool flag)
{
    
    int send_len = send(client.Get_Fd(),send_msg.c_str(),send_msg.length(),0);
    if(!flag){
        int len = client.Recv(sizeof(Proto_Head));
        //std::cout<< len <<std::endl;
        Proto_Head head;
        memset(&head,0,sizeof(Proto_Head));
        Decode_Proto_Head(string(client.Get_Read_Buffer()),head);
        client.Clean_Read_Buffer();
        len = client.Recv(head.length);
        //std::cout<<"body len : "<<len <<std::endl;
    }
    if(client.Get_Read_Buffer().compare(except)==0){
        printf("%s : Success\n",func);
    }else{
        printf("%s : Failed\n",func);
        printf("failue return : %s %ld %ld\n",client.Get_Read_Buffer().cbegin(),client.Get_Read_Buffer().size(),except.length());
    }
    client.Clean_Read_Buffer();
}

string Proc_Protocol(string content)
{
    Proto_Head head;
    head.length = content.length();
    memcpy(head.version,"1.0",VERSION_LEN);
    char* buffer = new char[sizeof(Proto_Head)];
    memset(buffer,0,sizeof(Proto_Head));
    memcpy(buffer,&head,sizeof(Proto_Head));
    string res(buffer,sizeof(Proto_Head));
    return res+content;
}
int main(int argc,char *argv[])
{
    if(argc<2) exit(0);
    Client_Socket client(SERVER_IP,atoi(argv[1]));
    client.Socket_Init();
    client.Connect();
    Test(client,Proc_Protocol("SET str 'my best girl friend'"),"OK\r\n","SET",false);
    Test(client,Proc_Protocol("LEN str"),"19\r\n","LEN",false);
    Test(client,Proc_Protocol("APPAND str ' is lyj'"),"OK\r\n","APPAND",false);
    Test(client,Proc_Protocol("GET str"),"my best girl friend is lyj\r\n","GET",false);
    Test(client,Proc_Protocol("EXIST str"),"EXIST\r\n","EXIST",false);
    Test(client,Proc_Protocol("EXIST name "),"NOT EXIST\r\n","NOT EXIST",false);
    Test(client,Proc_Protocol("DELETE str"),"OK\r\n","DELETE",false);
    
    Test(client,Proc_Protocol("ASET name king"),"OK\r\n","ASET ONE",false);
    Test(client,Proc_Protocol("ASET name Darren gzj lyj gzb"),"OK\r\n","ASET More",false);
    Test(client,Proc_Protocol("AGET name"),"king\r\nDarren\r\ngzj\r\nlyj\r\ngzb\r\n","AGET",false);
    Test(client,Proc_Protocol("ACOUNT name"),"5\r\n","ACOUNT",false);
    Test(client,Proc_Protocol("AEXIST name king"),"EXIST\r\n","AEXIST",false);
    Test(client,Proc_Protocol("AEXIST name GGBond"),"NOT EXIST\r\n","NOT AEXIST",false);
    Test(client,Proc_Protocol("ADELETE name gzb"),"1\r\n","ADELETE ONE",false);
    Test(client,Proc_Protocol("ACOUNT name"),"4\r\n","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("ADELETE name gzj lyj"),"2\r\n","ADELETE MORE",false);
    Test(client,Proc_Protocol("ACOUNT name"),"2\r\n","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("ADELETE name"),"2\r\n","ADELETE ALL",false);
    Test(client,Proc_Protocol("ACOUNT name"),"NO KEY\r\n","VERIFY LAST COMMAND",false);
    
    Test(client,Proc_Protocol("LPUSH test world hello"),"OK\r\n","LPUSH",false);
    Test(client,Proc_Protocol("LGET test"),"hello\r\nworld\r\n","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("RPUSH test my name"),"OK\r\n","RPUSH",false);
    Test(client,Proc_Protocol("LGET test"),"hello\r\nworld\r\nmy\r\nname\r\n","LGET",false);
    Test(client,Proc_Protocol("LCOUNT test"),"4\r\n","LCOUNT",false);
    Test(client,Proc_Protocol("LEXIST test my"),"EXIST\r\n","LEXIST",false);
    Test(client,Proc_Protocol("LEXIST test GGBond"),"NOT EXIST\r\n","NOT LEXIST",false);
    Test(client,Proc_Protocol("LDELETE test hello"),"1\r\n","LDELETE ONE",false);
    Test(client,Proc_Protocol("LCOUNT test"),"3\r\n","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("LDELETE test my name"),"2\r\n","LDELETE MORE",false);
    Test(client,Proc_Protocol("LCOUNT test"),"1\r\n","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("LDELETE test"),"1\r\n","LDELETE ALL",false);
    Test(client,Proc_Protocol("LCOUNT test"),"NO KEY\r\n","VERIFY LAST COMMAND",false);

    Test(client,Proc_Protocol("RSET key girl lyj"),"OK\r\n","RSET ONE",false);
    Test(client,Proc_Protocol("RSET key boy gzj animal cat food milk house tcyp"),"OK\r\n","RSET MORE",false);
    Test(client,Proc_Protocol("RGET key girl"),"lyj\r\n","RGET",false);
    Test(client,Proc_Protocol("RCOUNT key"),"5\r\n","RCOUNT",false);
    Test(client,Proc_Protocol("REXIST key girl"),"EXIST\r\n","REXIST",false);
    Test(client,Proc_Protocol("REXIST key sun"),"NOT EXIST\r\n","NOT REXIST",false);
    Test(client,Proc_Protocol("RDELETE key animal"),"1\r\n","RDELETE ONE",false);
    Test(client,Proc_Protocol("RCOUNT key"),"4\r\n","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("RDELETE key house food"),"2\r\n","RDELETE MORE",false);
    Test(client,Proc_Protocol("RCOUNT key"),"2\r\n","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("RDELETE key"),"2\r\n","RDELETE ALL",false);
    Test(client,Proc_Protocol("RCOUNT key"),"NO KEY\r\n","VERIFY LAST COMMAND",false);
    
    Test(client,Proc_Protocol("SSET name king"),"OK\r\n","SSET ONE",false);
    Test(client,Proc_Protocol("SSET name Darren gzj lyj gzb"),"OK\r\n","SSET More",false);
    Test(client,Proc_Protocol("SGET name"),"king\r\nDarren\r\ngzj\r\nlyj\r\ngzb\r\n","SGET",false);
    Test(client,Proc_Protocol("SCOUNT name"),"5\r\n","SCOUNT",false);
    Test(client,Proc_Protocol("SEXIST name king"),"EXIST\r\n","SEXIST",false);
    Test(client,Proc_Protocol("SEXIST name GGBond"),"NOT EXIST\r\n","NOT SEXIST",false);
    Test(client,Proc_Protocol("SDELETE name gzb"),"1\r\n","SDELETE ONE",false);
    Test(client,Proc_Protocol("SCOUNT name"),"4\r\n","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("SDELETE name gzj lyj"),"2\r\n","SDELETE MORE",false);
    Test(client,Proc_Protocol("SCOUNT name"),"2\r\n","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("SDELETE name"),"2\r\n","SDELETE ALL",false);
    Test(client,Proc_Protocol("SCOUNT name"),"NO KEY\r\n","VERIFY LAST COMMAND",false);



    Test(client,Proc_Protocol("SET name lu"),"OK\r\n","Before affairs",false);
    Test(client,Proc_Protocol("BEG"),"","BEG AFFAIRS",true);
    Test(client,Proc_Protocol("APPAND name ' yv jia'"),"","ADDING",true);
    Test(client,Proc_Protocol("SET teacher king"),"","ADDING",true);
    Test(client,Proc_Protocol("END"),"OK\r\n","END AFFAIRS",false);
    Test(client,Proc_Protocol("GET name"),"lu yv jia\r\n","IDENTIFY",false);
    Test(client,Proc_Protocol("GET teacher"),"king\r\n","IDENTIFY",false);
    Test(client,Proc_Protocol("ROLLBACK"),"OK\r\n","ROLLBACK",false);
    Test(client,Proc_Protocol("GET name"),"lu\r\n","IDENTIFY",false);
    Test(client,Proc_Protocol("GET teacher"),"NO KEY\r\n","IDENTIFY",false);
    Test(client,Proc_Protocol("CLEAN_CACHE"),"OK\r\n","CLEAN",false);
    Test(client,Proc_Protocol("ROLLBACK"),"OK\r\n","IDENTIFY",false);
    Test(client,Proc_Protocol("GET name"),"NO KEY\r\n","IDENTIFY",false);
    client.Close();
    return 0;
}