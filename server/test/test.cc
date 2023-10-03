/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-29 05:40:03
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-03 12:02:03
 */
#include<iostream>
#include<netinet/in.h>
#include<string.h>
#include"Client_Socket/Client_Socket.hpp"
using namespace Client_Socket_NSP;
void Test(Client_Socket& client ,string send_msg,const char *except,const char* func)
{
    int msg_len = send_msg.length();
    msg_len = htonl(msg_len);
    send(client.Get_Fd(),&msg_len,sizeof(msg_len),0);
    int send_len = send(client.Get_Fd(),send_msg.c_str(),send_msg.length(),0);
    std::cout<<"send len:"<<send_len<<std::endl;
    int len = client.Recv();
    if(client.Get_Read_Buffer().compare(except)==0){
        printf("%s : Success\n",func);
    }else{
        printf("%s : Failed\n",func);
        printf("failue return : %s %ld %ld\n",client.Get_Read_Buffer().cbegin(),client.Get_Read_Buffer().size(),strlen(except));
    }
    client.Clean_Read_Buffer();
}

string Proc_Protocol(string content)
{
    
    return content;
}



int main(int argc,char *argv[])
{
    if(argc<2) exit(0);
    Client_Socket client("192.168.124.140",atoi(argv[1]));
    client.Socket_Init();
    client.Connect();
    Test(client,Proc_Protocol("SET str 'my best girl friend'"),"OK\r\n","SET");
    Test(client,Proc_Protocol("LEN str"),"19\r\n","LEN");
    Test(client,Proc_Protocol("APPAND str ' is lyj'"),"OK\r\n","APPAND");
    Test(client,Proc_Protocol("GET str"),"my best girl friend is lyj\r\n","GET");
    Test(client,Proc_Protocol("EXIST str"),"EXIST\r\n","EXIST");
    Test(client,Proc_Protocol("EXIST name "),"NOT EXIST\r\n","NOT EXIST");
    Test(client,Proc_Protocol("DELETE str"),"OK\r\n","DELETE");
    
    Test(client,Proc_Protocol("ASET name king"),"OK\r\n","ASET ONE");
    Test(client,Proc_Protocol("ASET name Darren gzj lyj gzb"),"OK\r\n","ASET More");
    Test(client,Proc_Protocol("AGET name"),"king\r\nDarren\r\ngzj\r\nlyj\r\ngzb\r\n","AGET");
    Test(client,Proc_Protocol("ACOUNT name"),"5\r\n","ACOUNT");
    Test(client,Proc_Protocol("AEXIST name king"),"EXIST\r\n","AEXIST");
    Test(client,Proc_Protocol("AEXIST name GGBond"),"NOT EXIST\r\n","NOT AEXIST");
    Test(client,Proc_Protocol("ADELETE name gzb"),"1\r\n","ADELETE ONE");
    Test(client,Proc_Protocol("ACOUNT name"),"4\r\n","VERIFY LAST COMMAND");
    Test(client,Proc_Protocol("ADELETE name gzj lyj"),"2\r\n","ADELETE MORE");
    Test(client,Proc_Protocol("ACOUNT name"),"2\r\n","VERIFY LAST COMMAND");
    Test(client,Proc_Protocol("ADELETE name"),"2\r\n","ADELETE ALL");
    Test(client,Proc_Protocol("ACOUNT name"),"NO KEY\r\n","VERIFY LAST COMMAND");
    
    Test(client,Proc_Protocol("LPUSH test world hello"),"OK\r\n","LPUSH");
    Test(client,Proc_Protocol("LGET test"),"hello\r\nworld\r\n","VERIFY LAST COMMAND");
    Test(client,Proc_Protocol("RPUSH test my name"),"OK\r\n","RPUSH");
    Test(client,Proc_Protocol("LGET test"),"hello\r\nworld\r\nmy\r\nname\r\n","LGET");
    Test(client,Proc_Protocol("LCOUNT test"),"4\r\n","LCOUNT");
    Test(client,Proc_Protocol("LEXIST test my"),"EXIST\r\n","LEXIST");
    Test(client,Proc_Protocol("LEXIST test GGBond"),"NOT EXIST\r\n","NOT LEXIST");
    Test(client,Proc_Protocol("LDELETE test hello"),"1\r\n","LDELETE ONE");
    Test(client,Proc_Protocol("LCOUNT test"),"3\r\n","VERIFY LAST COMMAND");
    Test(client,Proc_Protocol("LDELETE test my name"),"2\r\n","LDELETE MORE");
    Test(client,Proc_Protocol("LCOUNT test"),"1\r\n","VERIFY LAST COMMAND");
    Test(client,Proc_Protocol("LDELETE test"),"1\r\n","LDELETE ALL");
    Test(client,Proc_Protocol("LCOUNT test"),"NO KEY\r\n","VERIFY LAST COMMAND");

    Test(client,Proc_Protocol("RSET key girl lyj"),"OK\r\n","RSET ONE");
    Test(client,Proc_Protocol("RSET key boy gzj animal cat food milk house tcyp"),"OK\r\n","RSET MORE");
    Test(client,Proc_Protocol("RGET key girl"),"lyj\r\n","RGET");
    Test(client,Proc_Protocol("RCOUNT key"),"5\r\n","RCOUNT");
    Test(client,Proc_Protocol("REXIST key girl"),"EXIST\r\n","REXIST");
    Test(client,Proc_Protocol("REXIST key sun"),"NOT EXIST\r\n","NOT REXIST");
    Test(client,Proc_Protocol("RDELETE key animal"),"1\r\n","RDELETE ONE");
    Test(client,Proc_Protocol("RCOUNT key"),"4\r\n","VERIFY LAST COMMAND");
    Test(client,Proc_Protocol("RDELETE key house food"),"2\r\n","RDELETE MORE");
    Test(client,Proc_Protocol("RCOUNT key"),"2\r\n","VERIFY LAST COMMAND");
    Test(client,Proc_Protocol("RDELETE key"),"2\r\n","RDELETE ALL");
    Test(client,Proc_Protocol("RCOUNT key"),"NO KEY\r\n","VERIFY LAST COMMAND");
    
    Test(client,Proc_Protocol("SSET name king"),"OK\r\n","SSET ONE");
    Test(client,Proc_Protocol("SSET name Darren gzj lyj gzb"),"OK\r\n","SSET More");
    Test(client,Proc_Protocol("SGET name"),"king\r\nDarren\r\ngzj\r\nlyj\r\ngzb\r\n","SGET");
    Test(client,Proc_Protocol("SCOUNT name"),"5\r\n","SCOUNT");
    Test(client,Proc_Protocol("SEXIST name king"),"EXIST\r\n","SEXIST");
    Test(client,Proc_Protocol("SEXIST name GGBond"),"NOT EXIST\r\n","NOT SEXIST");
    Test(client,Proc_Protocol("SDELETE name gzb"),"1\r\n","SDELETE ONE");
    Test(client,Proc_Protocol("SCOUNT name"),"4\r\n","VERIFY LAST COMMAND");
    Test(client,Proc_Protocol("SDELETE name gzj lyj"),"2\r\n","SDELETE MORE");
    Test(client,Proc_Protocol("SCOUNT name"),"2\r\n","VERIFY LAST COMMAND");
    Test(client,Proc_Protocol("SDELETE name"),"2\r\n","SDELETE ALL");
    Test(client,Proc_Protocol("SCOUNT name"),"NO KEY\r\n","VERIFY LAST COMMAND");


    client.Close();
    return 0;
}