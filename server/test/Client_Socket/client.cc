#include<iostream>
#include<string>
#include"Client_Socket.hpp"
using namespace std;
using namespace Client_Socket_NSP;
int main()
{
    Client_Socket client("127.0.0.1",8080);
    client.Socket_Init();
    client.Connect();
    string a="hello world";
    client.Set_Write_Buffer(a);
    int len = client.Send();
    cout<<len<<endl;
    client.Recv();
    cout<<client.Get_Read_Buffer();
    return 0;
}