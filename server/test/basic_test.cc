/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-29 05:40:03
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-12 13:05:56
 */
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include "netlib/netlib.h"
#define VERSION_LEN 4
#define SERVER_IP "192.168.124.140"
using namespace std;
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


void Test(Client_Sock& client ,string send_msg,string except,const char* func, bool flag)
{
    
    int send_len = client.Send(send_msg);
    string buffer = "";
    if(!flag){
        int len = client.Recv(buffer,sizeof(Proto_Head));
        //std::cout<< len <<std::endl;
        Proto_Head head;
        memset(&head,0,sizeof(Proto_Head));
        Decode_Proto_Head(buffer,head);
        len = client.Recv(buffer,head.length);
        //std::cout<<"body len : "<<len <<std::endl;
    }
    if(buffer.compare(except)==0){
        printf("%s : Success\n",func);
    }else{
        printf("%s : Failed\n",func);
        printf("failue return : %s %ld %ld\n",buffer.c_str(),buffer.size(),except.length());
    }
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
    Client_Sock client;
    int fd = client.Sock();
    client.Connect(SERVER_IP,atoi(argv[1]));
    Test(client,Proc_Protocol("SET str 'my best girl friend'"),"OK","SET",false);
    Test(client,Proc_Protocol("LEN str"),"19","LEN",false);
    Test(client,Proc_Protocol("APPAND str ' is lyj'"),"OK","APPAND",false);
    Test(client,Proc_Protocol("GET str"),"my best girl friend is lyj","GET",false);
    Test(client,Proc_Protocol("EXIST str"),"EXIST","EXIST",false);
    Test(client,Proc_Protocol("EXIST name "),"NOT EXIST","NOT EXIST",false);
    Test(client,Proc_Protocol("DELETE str"),"OK","DELETE",false);
    
    Test(client,Proc_Protocol("ASET name king"),"OK","ASET ONE",false);
    Test(client,Proc_Protocol("ASET name Darren gzj lyj gzb"),"OK","ASET More",false);
    Test(client,Proc_Protocol("AGET name"),"king\nDarren\ngzj\nlyj\ngzb","AGET",false);
    Test(client,Proc_Protocol("ACOUNT name"),"5","ACOUNT",false);
    Test(client,Proc_Protocol("AEXIST name king"),"EXIST","AEXIST",false);
    Test(client,Proc_Protocol("AEXIST name GGBond"),"NOT EXIST","NOT AEXIST",false);
    Test(client,Proc_Protocol("ADELETE name gzb"),"1","ADELETE ONE",false);
    Test(client,Proc_Protocol("ACOUNT name"),"4","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("ADELETE name gzj lyj"),"2","ADELETE MORE",false);
    Test(client,Proc_Protocol("ACOUNT name"),"2","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("ADELETE name"),"2","ADELETE ALL",false);
    Test(client,Proc_Protocol("ACOUNT name"),"NO KEY","VERIFY LAST COMMAND",false);
    
    Test(client,Proc_Protocol("LPUSH test world hello"),"OK","LPUSH",false);
    Test(client,Proc_Protocol("LGET test"),"hello\nworld\n","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("RPUSH test my name"),"OK","RPUSH",false);
    Test(client,Proc_Protocol("LGET test"),"hello\nworld\nmy\nname","LGET",false);
    Test(client,Proc_Protocol("LCOUNT test"),"4","LCOUNT",false);
    Test(client,Proc_Protocol("LEXIST test my"),"EXIST","LEXIST",false);
    Test(client,Proc_Protocol("LEXIST test GGBond"),"NOT EXIST","NOT LEXIST",false);
    Test(client,Proc_Protocol("LDELETE test hello"),"1","LDELETE ONE",false);
    Test(client,Proc_Protocol("LCOUNT test"),"3","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("LDELETE test my name"),"2","LDELETE MORE",false);
    Test(client,Proc_Protocol("LCOUNT test"),"1","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("LDELETE test"),"1","LDELETE ALL",false);
    Test(client,Proc_Protocol("LCOUNT test"),"NO KEY","VERIFY LAST COMMAND",false);

    Test(client,Proc_Protocol("RSET key girl lyj"),"OK","RSET ONE",false);
    Test(client,Proc_Protocol("RSET key boy gzj animal cat food milk house tcyp"),"OK","RSET MORE",false);
    Test(client,Proc_Protocol("RGET key girl"),"lyj","RGET",false);
    Test(client,Proc_Protocol("RCOUNT key"),"5","RCOUNT",false);
    Test(client,Proc_Protocol("REXIST key girl"),"EXIST","REXIST",false);
    Test(client,Proc_Protocol("REXIST key sun"),"NOT EXIST","NOT REXIST",false);
    Test(client,Proc_Protocol("RDELETE key animal"),"1","RDELETE ONE",false);
    Test(client,Proc_Protocol("RCOUNT key"),"4","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("RDELETE key house food"),"2","RDELETE MORE",false);
    Test(client,Proc_Protocol("RCOUNT key"),"2","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("RDELETE key"),"2","RDELETE ALL",false);
    Test(client,Proc_Protocol("RCOUNT key"),"NO KEY","VERIFY LAST COMMAND",false);
    
    Test(client,Proc_Protocol("SSET name king"),"OK","SSET ONE",false);
    Test(client,Proc_Protocol("SSET name Darren gzj lyj gzb"),"OK","SSET More",false);
    Test(client,Proc_Protocol("SGET name"),"king\nDarren\ngzj\nlyj\ngzb","SGET",false);
    Test(client,Proc_Protocol("SCOUNT name"),"5","SCOUNT",false);
    Test(client,Proc_Protocol("SEXIST name king"),"EXIST","SEXIST",false);
    Test(client,Proc_Protocol("SEXIST name GGBond"),"NOT EXIST","NOT SEXIST",false);
    Test(client,Proc_Protocol("SDELETE name gzb"),"1","SDELETE ONE",false);
    Test(client,Proc_Protocol("SCOUNT name"),"4","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("SDELETE name gzj lyj"),"2","SDELETE MORE",false);
    Test(client,Proc_Protocol("SCOUNT name"),"2","VERIFY LAST COMMAND",false);
    Test(client,Proc_Protocol("SDELETE name"),"2","SDELETE ALL",false);
    Test(client,Proc_Protocol("SCOUNT name"),"NO KEY","VERIFY LAST COMMAND",false);



    Test(client,Proc_Protocol("SET name lu"),"OK","Before affairs",false);
    Test(client,Proc_Protocol("BEG"),"","BEG AFFAIRS",true);
    Test(client,Proc_Protocol("APPAND name ' yv jia'"),"","ADDING",true);
    Test(client,Proc_Protocol("SET teacher king"),"","ADDING",true);
    Test(client,Proc_Protocol("END"),"OK","END AFFAIRS",false);
    Test(client,Proc_Protocol("GET name"),"lu yv jia","IDENTIFY",false);
    Test(client,Proc_Protocol("GET teacher"),"king","IDENTIFY",false);
    Test(client,Proc_Protocol("ROLLBACK"),"OK","ROLLBACK",false);
    Test(client,Proc_Protocol("GET name"),"lu","IDENTIFY",false);
    Test(client,Proc_Protocol("GET teacher"),"NO KEY","IDENTIFY",false);
    Test(client,Proc_Protocol("CLEAN_CACHE"),"OK","CLEAN",false);
    Test(client,Proc_Protocol("ROLLBACK"),"OK","IDENTIFY",false);
    Test(client,Proc_Protocol("GET name"),"NO KEY","IDENTIFY",false);
    client.Close();
    return 0;
}