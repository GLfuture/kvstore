#include "app.h"
#include <cstdint>
void Accept_cb(Reactor R, Server_Ptr server)
{
    int clinetfd = server->Accept();
    R.Add_Reactor(clinetfd, EPOLLIN);
}

void Read_cb(APP *app , Reactor R, Server_Ptr server, ThreadPool &th_pool)
{
    int clientfd = R.Get_Now_Event().data.fd;
    int len = server->Recv(clientfd);
    if(len <= 0){
        R.Del_Reactor(clientfd,EPOLLIN);
        server->Close(clientfd);
        return;
    }
    Client_Ptr client = server->Get_Client(clientfd);
    string rbuffer = client->Get_Rbuffer();
    client->Clean_Rbuffer();
    std::future<string> res=th_pool.exec(std::bind(&APP::Work,app,rbuffer));
    std::shared_future<string> share_res=res.share();
    app->Result_Add_Future(clientfd,share_res);
    R.Mod_Reactor(clientfd,EPOLLOUT);
}

void Write_cb(APP *app , Reactor R, Server_Ptr server, ThreadPool &th_pool)
{
    int clientfd = R.Get_Now_Event().data.fd;
    Client_Ptr client = server->Get_Client(clientfd);
    string res=app->Get_An_Finished_Result();
    if(!res.empty()){
        client->Set_Wbuffer(res);
        server->Send(clientfd);
    }
    R.Mod_Reactor(clientfd,EPOLLIN);
}

int APP::Init(uint16_t port,uint16_t backlog,uint8_t thread_num,uint32_t buffer_size)
{
    th_pool.Create(thread_num);
    Server_Ptr server = R.Get_Server();
    if (server->Init_Sock(port, backlog) == -1)
        return -1;
    R.Set_No_Block(server->Get_Sock());
    R.Set_Accept_cb(std::bind(Accept_cb, R, server));
    R.Set_Read_cb(std::bind(Read_cb, this, R, server, std::ref(th_pool)));
    R.Set_Write_cb(std::bind(Write_cb, this, R, server, std::ref(th_pool)));
    R.Add_Reactor(server->Get_Sock(),EPOLLIN);
    return 0;
}

void APP::Result_Add_Future(int clientfd,std::shared_future<string>& res_ptr)
{
    
    this->result[clientfd].push(res_ptr);
}


string APP::Decode(string buffer)
{
    Split split(buffer);
    if(!split.Spilt_With_Char_Connect_With_Quote(' ')){
        return ret_msg[RET_STR_ERROR];
    }
    vector<string_view> res = split.Get_Result();
    string ret_info="";
    int i;
    for(i = CMD_BEG ; i < CMD_END ; i++)
    {
        if(res[0].compare(cmds[i]) == 0){
            if(i <= CMD_EXIST)
                ret_info = Decode_String(i,res);
            else if(i<=CMD_AEXIST)
                ret_info = Decode_Array(i,res);
            else if(i<=CMD_LEXIST)
                ret_info = Decode_List(i,res);
            else if(i<=CMD_REXIST)
                ret_info = Decode_Rbtree(i,res);
            else if(i<=CMD_SEXIST)
                ret_info = Decode_Set(i,res);
            //else if(i<=CMD_ZEXIST)
                //ret_info = Decode_Skiptable(i,res);
            break;
        }
    }
    if(i==CMD_END) ret_info=ret_msg[RET_CMD_ERROR];
    return ret_info;
}

//返回一个执行完毕的结果
string APP::Get_An_Finished_Result()
{
    string ret_info;
    for(map<int16_t,queue<std::shared_future<string>>>::iterator it=result.begin();it!=result.end();it++)
    {
        //异步运行完毕
        while(!(*it).second.empty() && (*it).second.front().valid())
        {
            ret_info = (*it).second.front().get();
            (*it).second.pop();
            return ret_info;
        }
    }
    return ret_info;
}


string APP::Work(string rbuffer)
{
    string res = Decode(rbuffer);

    return res;
}

string APP::Decode_String(int cmd_type, vector<string_view> &res)
{
    string ret_info;
    switch (cmd_type)
    {
        case CMD_SET:
        {
            if(res.size() != 3)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_Set(string(res[1]),string(res[2]));
            return ret_info;
        }
        case CMD_GET:
        {
            if(res.size()!=2)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_Get(string(res[1])); 
            return ret_info;
        }
        case CMD_APPAND:
        {
            if(res.size()!=3)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_Appand(string(res[1]),string(res[2]));
            return ret_info;
        }
        case CMD_LEN:
        {
            if(res.size() != 2)  return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_Len(string(res[1]));
            return ret_info;
        }
        case CMD_DELETE:
        {
            if(res.size() != 2)  return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_Delete(string(res[1]));
            return ret_info;
        }
        case CMD_EXIST:
        {
            if(res.size() != 2)  return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_Exist(string(res[1]));
            return ret_info;
        }
        default:
            break;
    }
    return ret_info;
}

string APP::Decode_Array(int cmd_type, vector<string_view> &res)
{
    string ret_info;
    switch (cmd_type)
    {
        case CMD_ASET:
        {
            if(res.size()<3)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_ASet(string(res[1]),res);
            return ret_info;
        }
        case CMD_AGET:
        {
            if(res.size()!=2)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_AGet(string(res[1])); 
            return ret_info;
        }
        case CMD_ACOUNT:
        {
            if(res.size()!=2)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_ACount(string(res[1]));
            return ret_info;
        }
        case CMD_ADELETE:
        {
            if(res.size() < 2)  return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_ADelete(string(res[1]),res);
            return ret_info;
        }
        case CMD_AEXIST:
        {
            if(res.size() != 3)  return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_AExist(string(res[1]),string(res[2]));
            return ret_info;
        }
        default:
            break;
    }
    return ret_info;
}

string APP::Decode_List(int cmd_type, vector<string_view> &res)
{
    string ret_info;
    switch (cmd_type)
    {
        case CMD_LPUSH:
        {
            if(res.size()<3)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_LPUSH(string(res[1]),res);
            return ret_info;
        }
        case CMD_RPUSH:
        {
            if(res.size()<3)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_RPUSH(string(res[1]),res);
            return ret_info;
        }
        case CMD_LGET:
        {
            if(res.size()!=2)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_LGet(string(res[1])); 
            return ret_info;
        }
        case CMD_LCOUNT:
        {
            if(res.size()!=2)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_LCount(string(res[1]));
            return ret_info;
        }
        case CMD_LDELETE:
        {
            if(res.size() < 2)  return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_LDelete(string(res[1]),res);
            return ret_info;
        }
        case CMD_LEXIST:
        {
            if(res.size() != 3)  return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_LExist(string(res[1]),string(res[2]));
            return ret_info;
        }
        default:
            break;
    }
    return ret_info;
}

string APP::Decode_Rbtree(int cmd_type, vector<string_view> &res)
{
    string ret_info;
    switch (cmd_type)
    {
        case CMD_RSET:
        {
            if (res.size() < 4 || res.size() % 2 != 0)
            return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_RSet(string(res[1]), res);
            return ret_info;
        }
        case CMD_RGET:
        {
            if (res.size() != 3)
            return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_RGet(string(res[1]),string(res[2]));;
            return ret_info;
        }
        case CMD_RCOUNT:
        {
            if (res.size() != 2)
            return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_RCount(string(res[1]));
            return ret_info;
        }
        case CMD_RDELETE:
        {
            if (res.size() < 2)
            return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_RDelete(string(res[1]),res);
            return ret_info;
        }
        case CMD_REXIST:
        {
            if (res.size() != 3)
            return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_RExist(string(res[1]), string(res[2]));
            return ret_info;
        }
        default:
            break;
    }
    return ret_info;
}

string APP::Decode_Set(int cmd_type, vector<string_view> &res)
{
    string ret_info;
    switch (cmd_type)
    {
        case CMD_SSET:
        {
            if(res.size()<3)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_SSet(string(res[1]),res);
            return ret_info;
        }
        case CMD_SGET:
        {
            if (res.size() != 2)
            return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_SGet(string(res[1]));;
            return ret_info;
        }
        case CMD_SCOUNT:
        {
            if (res.size() != 2)
            return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_SCount(string(res[1]));
            return ret_info;
        }
        case CMD_SDELETE:
        {
            if (res.size() < 2)
            return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_SDelete(string(res[1]),res);
            return ret_info;
        }
        case CMD_SEXIST:
        {
            if (res.size() != 3)
            return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_SExist(string(res[1]), string(res[2]));
            return ret_info;
        }
        default:
            break;
    }
    return ret_info;
}

string APP::Exec_Cmd_Set(string key, string value)
{
    string_store[key]=value;
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_Get(string key)
{
    map<string,string>::iterator it=string_store.find(key);
    if(it==string_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    return (*it).second+"\r\n";
}

string APP::Exec_Cmd_Appand(string key, string value)
{
    map<string,string>::iterator it=string_store.find(key);
    if(it==string_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    (*it).second.append(value);
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_Len(string key)
{
    map<string,string>::iterator it=string_store.find(key);
    if(it==string_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    return std::to_string((*it).second.length())+"\r\n";
}

string APP::Exec_Cmd_Delete(string key)
{
    map<string,string>::iterator it=string_store.find(key);
    if(it==string_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    string_store.erase(it);
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_Exist(string key)
{
    map<string,string>::iterator it=string_store.find(key);
    if(it==string_store.end()){
        return ret_msg[RET_NOT_EXIST];
    }
    return ret_msg[RET_EXIST];
}

string APP::Exec_Cmd_ASet(string key,vector<string_view> &res)
{
    for(int i=2;i<res.size();i++)
    {
        //这里不能使用empalce_back，临时对象会被释放
        array_store[key].push_back(string(res[i]));
    }
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_AGet(string key)
{
    map<string,vector<string>>::iterator it = array_store.find(key);
    //未找到key
    if( it == array_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    //使用span进行只读操作
    span<string> res((*it).second);
    string ret_value;
    for(int i=0;i<res.size();i++){
        ret_value = ret_value + res[i] + "\r\n";
    }
    return ret_value;
}

string APP::Exec_Cmd_ACount(string key)
{
    map<string,vector<string>>::iterator it = array_store.find(key);
    //未找到key
    if( it == array_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    return std::to_string((*it).second.size())+"\r\n";
}

string APP::Exec_Cmd_ADelete(string key,vector<string_view> &res)
{
    string ret_value;
    map<string,vector<string>>::iterator it = array_store.find(key);
    //未找到key
    if( it == array_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    int count = 0;
    if(res.size()==2){
        count = (*it).second.size();
        array_store.erase(it);
    }else{
        for (int i = 2; i < res.size(); i++)
        {
            for (vector<string>::iterator array_it = (*it).second.begin(); array_it != (*it).second.end(); array_it++)
            {
                if (res[i].compare(*array_it) == 0)
                {
                    array_it = (*it).second.erase(array_it);
                    count++;
                    break;
                }
            }
        }
    }
    return std::to_string(count)+"\r\n";
}

string APP::Exec_Cmd_AExist(string key, string value)
{
    map<string,vector<string>>::iterator it = array_store.find(key);
    if( it == array_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    for (vector<string>::iterator array_it = (*it).second.begin(); array_it != (*it).second.end(); array_it++)
    {
        if (value.compare(*array_it) == 0)
        {
            return ret_msg[RET_EXIST];
        }
    }
    return ret_msg[RET_NOT_EXIST];
}

string APP::Exec_Cmd_LPUSH(string key, vector<string_view> &res)
{
    //使用迭代器防止减少每次从红黑树中查询的操作
    for(int i=2;i<res.size();i++)
    {
        list_store[key].push_front(string(res[i]));
    }
    
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_RPUSH(string key, vector<string_view> &res)
{
    //使用迭代器防止减少每次从红黑树中查询的操作
    for(int i=2;i<res.size();i++)
    {
        list_store[key].push_back(string(res[i]));
    }
    
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_LGet(string key)
{
    map<string,list<string>>::iterator it = list_store.find(key);
    //未找到key
    if( it == list_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    string ret_value;
    for(list<string>::iterator list_it=(*it).second.begin();list_it!=(*it).second.end();list_it++)
    {
        ret_value=ret_value+(*list_it)+"\r\n";
    }
    return ret_value;
}

string APP::Exec_Cmd_LCount(string key)
{
    map<string,list<string>>::iterator it = list_store.find(key);
    //未找到key
    if( it == list_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    return std::to_string((*it).second.size())+"\r\n";
}

string APP::Exec_Cmd_LDelete(string key, vector<string_view> &res)
{
    map<string,list<string>>::iterator it = list_store.find(key);
    //未找到key
    if( it == list_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    int count=0;
    if(res.size()==2){
        count=(*it).second.size();
        list_store.erase(it);
    }else{
        for (int i = 2; i < res.size(); i++)
        {
            for (list<string>::iterator list_it = (*it).second.begin(); list_it != (*it).second.end(); list_it++)
            {
                if ((*list_it).compare(res[i]) == 0)
                {
                    list_it = (*it).second.erase(list_it);
                    count++;
                    break;
                }
            }
        }
    }
    return std::to_string(count)+"\r\n";
}

string APP::Exec_Cmd_LExist(string key, string value)
{
    map<string,list<string>>::iterator it = list_store.find(key);
    if( it == list_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    for (list<string>::iterator list_it = (*it).second.begin(); list_it != (*it).second.end(); list_it++)
    {
        if (value.compare(*list_it) == 0)
        {
            return ret_msg[RET_EXIST];
        }
    }
    return ret_msg[RET_NOT_EXIST];
}

string APP::Exec_Cmd_RSet(string key,vector<string_view> &res)
{
    //使用迭代器防止减少每次从红黑树中查询的操作
    for(int i=2;i<res.size();i+=2){
        rbtree_store[key][string(res[i])]=string(res[i+1]);
    }
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_RGet(string key,string field)
{
    map<string,map<string,string>>::iterator it=rbtree_store.find(key);
    if(it == rbtree_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    map<string,string>::iterator rb_it=(*it).second.find(field);
    if(rb_it == (*it).second.end()){
        return ret_msg[RET_NO_FIELD];
    }
    return (*rb_it).second + "\r\n";
}

string APP::Exec_Cmd_RCount(string key)
{
    map<string,map<string,string>>::iterator it=rbtree_store.find(key);
    if(it == rbtree_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    return std::to_string((*it).second.size())+"\r\n";
}

string APP::Exec_Cmd_RDelete(string key,vector<string_view> &res)
{
    map<string,map<string,string>>::iterator it=rbtree_store.find(key);
    if(it == rbtree_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    int count=0;
    if(res.size()==2){
        count=(*it).second.size();
        rbtree_store.erase(it);
    }else{
        for(int i=2;i<res.size();i++)
        {
            if((*it).second.find(string(res[i]))!=(*it).second.end()){
                (*it).second.erase(string(res[i]));
                count++;
            }
        }
    }
    return std::to_string(count)+"\r\n";
}

string APP::Exec_Cmd_RExist(string key, string field)
{
    map<string,map<string,string>>::iterator it=rbtree_store.find(key);
    if(it == rbtree_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    if((*it).second.find(field)==(*it).second.end()){
        return ret_msg[RET_NOT_EXIST];
    }
    return ret_msg[RET_EXIST];
}

string APP::Exec_Cmd_SSet(string key, vector<string_view> &res)
{
    for(int i=2;i<res.size();i++)
    {
        set_store[key].insert(string(res[i]));
    }
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_SGet(string key)
{
    map<string,unordered_set<string>>::iterator it = set_store.find(key);
    //未找到key
    if( it == set_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    string ret_value;
    for(unordered_set<string>::iterator set_it = (*it).second.begin();set_it!=(*it).second.end();set_it++){
        ret_value=ret_value+(*set_it)+"\r\n";
    }
    return ret_value;
}

string APP::Exec_Cmd_SCount(string key)
{
    map<string,unordered_set<string>>::iterator it = set_store.find(key);
    //未找到key
    if( it == set_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    return std::to_string((*it).second.size())+"\r\n";
}

string APP::Exec_Cmd_SDelete(string key, vector<string_view> &res)
{
    string ret_value;
    map<string,unordered_set<string>>::iterator it = set_store.find(key);
    //未找到key
    if( it == set_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    int count = 0;
    if(res.size()==2){
        count = (*it).second.size();
        set_store.erase(it);
    }else{
        unordered_set<string>::iterator set_it;
        for (int i = 2; i < res.size(); i++)
        {
            set_it=(*it).second.find(string(res[i]));
            if(set_it!=(*it).second.end()){
                set_it = (*it).second.erase(set_it);
                count++;
            }
        }
    }
    return std::to_string(count)+"\r\n";
}

string APP::Exec_Cmd_SExist(string key, string value)
{
    map<string,unordered_set<string>>::iterator it = set_store.find(key);
    if( it == set_store.end()){
        return ret_msg[RET_NO_KEY];
    }
    unordered_set<string>::iterator set_it = (*it).second.find(value);
    if(set_it==(*it).second.end()){
        return ret_msg[RET_NOT_EXIST];
    }
    return ret_msg[RET_EXIST];
}
