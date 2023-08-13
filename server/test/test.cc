#include<iostream>
#include<netinet/in.h>
#include<string.h>
#include"Client_Socket/Client_Socket.hpp"
using namespace Client_Socket_NSP;
void Test(Client_Socket& client ,const char *send_msg,const char *except,const char* func)
{
    client.Set_Write_Buffer(send_msg);
    client.Send();
    client.Clean_Send_Buffer();
    int len = client.Recv();
    if(client.Get_Read_Buffer().compare(except)==0){
        printf("%s : Success\n",func);
    }else{
        printf("%s : Failed\n",func);
        printf("failue return : %s %ld %ld\n",client.Get_Read_Buffer().cbegin(),client.Get_Read_Buffer().size(),strlen(except));
    }
    client.Clean_Read_Buffer();
}



int main(int argc,char *argv[])
{
    if(argc<2) exit(0);
    Client_Socket client("192.168.124.136",atoi(argv[1]));
    client.Socket_Init();
    client.Connect();
    Test(client,"SET str 'my best girl friend'","OK\r\n","SET");
    Test(client,"LEN str","19\r\n","LEN");
    Test(client,"APPAND str ' is lyj'","OK\r\n","APPAND");
    Test(client,"GET str","my best girl friend is lyj\r\n","GET");
    Test(client,"EXIST str","EXIST\r\n","EXIST");
    Test(client,"EXIST name ","NOT EXIST\r\n","NOT EXIST");
    Test(client,"DELETE str","OK\r\n","DELETE");


    Test(client,"ASET name king","OK\r\n","ASET ONE");
    Test(client,"ASET name Darren gzj lyj gzb","OK\r\n","ASET More");
    Test(client,"AGET name","king\r\nDarren\r\ngzj\r\nlyj\r\ngzb\r\n","AGET");
    Test(client,"ACOUNT name","5\r\n","ACOUNT");
    Test(client,"AEXIST name king","EXIST\r\n","AEXIST");
    Test(client,"AEXIST name GGBond","NOT EXIST\r\n","NOT AEXIST");
    Test(client,"ADELETE name gzb","1\r\n","ADELETE ONE");
    Test(client,"ACOUNT name","4\r\n","VERIFY LAST COMMAND");
    Test(client,"ADELETE name gzj lyj","2\r\n","ADELETE MORE");
    Test(client,"ACOUNT name","2\r\n","VERIFY LAST COMMAND");
    Test(client,"ADELETE name","2\r\n","ADELETE ALL");
    Test(client,"ACOUNT name","NO KEY\r\n","VERIFY LAST COMMAND");
    
    Test(client,"LPUSH test world hello","OK\r\n","LPUSH");
    Test(client,"LGET test","hello\r\nworld\r\n","VERIFY LAST COMMAND");
    Test(client,"RPUSH test my name","OK\r\n","RPUSH");
    Test(client,"LGET test","hello\r\nworld\r\nmy\r\nname\r\n","LGET");
    Test(client,"LCOUNT test","4\r\n","LCOUNT");
    Test(client,"LEXIST test my","EXIST\r\n","LEXIST");
    Test(client,"LEXIST test GGBond","NOT EXIST\r\n","NOT LEXIST");
    Test(client,"LDELETE test hello","1\r\n","LDELETE ONE");
    Test(client,"LCOUNT test","3\r\n","VERIFY LAST COMMAND");
    Test(client,"LDELETE test my name","2\r\n","LDELETE MORE");
    Test(client,"LCOUNT test","1\r\n","VERIFY LAST COMMAND");
    Test(client,"LDELETE test","1\r\n","LDELETE ALL");
    Test(client,"LCOUNT test","NO KEY\r\n","VERIFY LAST COMMAND");

    Test(client,"RSET key girl lyj","OK\r\n","RSET ONE");
    Test(client,"RSET key boy gzj animal cat food milk house tcyp","OK\r\n","RSET MORE");
    Test(client,"RGET key girl","lyj\r\n","RGET");
    Test(client,"RCOUNT key","5\r\n","RCOUNT");
    Test(client,"REXIST key girl","EXIST\r\n","REXIST");
    Test(client,"REXIST key sun","NOT EXIST\r\n","NOT REXIST");
    Test(client,"RDELETE key animal","1\r\n","RDELETE ONE");
    Test(client,"RCOUNT key","4\r\n","VERIFY LAST COMMAND");
    Test(client,"RDELETE key house food","2\r\n","RDELETE MORE");
    Test(client,"RCOUNT key","2\r\n","VERIFY LAST COMMAND");
    Test(client,"RDELETE key","2\r\n","RDELETE ALL");
    Test(client,"RCOUNT key","NO KEY\r\n","VERIFY LAST COMMAND");
    
    Test(client,"SSET name king","OK\r\n","SSET ONE");
    Test(client,"SSET name Darren gzj lyj gzb","OK\r\n","SSET More");
    Test(client,"SGET name","king\r\nDarren\r\ngzj\r\nlyj\r\ngzb\r\n","SGET");
    Test(client,"SCOUNT name","5\r\n","SCOUNT");
    Test(client,"SEXIST name king","EXIST\r\n","SEXIST");
    Test(client,"SEXIST name GGBond","NOT EXIST\r\n","NOT SEXIST");
    Test(client,"SDELETE name gzb","1\r\n","SDELETE ONE");
    Test(client,"SCOUNT name","4\r\n","VERIFY LAST COMMAND");
    Test(client,"SDELETE name gzj lyj","2\r\n","SDELETE MORE");
    Test(client,"SCOUNT name","2\r\n","VERIFY LAST COMMAND");
    Test(client,"SDELETE name","2\r\n","SDELETE ALL");
    Test(client,"SCOUNT name","NO KEY\r\n","VERIFY LAST COMMAND");


    client.Close();
    return 0;
}