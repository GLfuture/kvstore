#include "app/app.h"
#include <cstdint>

/**
 * @description: 接收连接触发回调
 * @param {Reactor} *R
 * @param {Server_Ptr} server
 * @return void
 * @author: Gong
 */
void Accept_cb(Reactor *R, APP::Server_Ptr server)
{
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
void Read_cb(APP *app , Reactor *R, APP::Server_Ptr server, ThreadPool &th_pool)
{
    int clientfd = R->Get_Now_Event().data.fd;
    APP::Tcp_Conn_Ptr conn = std::dynamic_pointer_cast<Tcp_Conn>(server->Get_Conn(clientfd));
    int proto_len = 0;
    int len = server->Recv_Len<int>(conn,proto_len);
    if (len <= 0)
    {
        R->Del_Reactor(clientfd, EPOLLIN);
        server->Close(clientfd);
        return;
    }
    proto_len=ntohl(proto_len);
    len = server->Recv(conn,proto_len);
    if(len <= 0){
        R->Del_Reactor(clientfd,EPOLLIN);
        server->Close(clientfd);
        return;
    }
    std::future<string> res = th_pool.exec(std::bind(&APP::Work,app,conn));
    std::shared_future<string> share_res=res.share();
    conn->Add_Future(share_res);
    R->Mod_Reactor(clientfd,EPOLLOUT);
}

//定时触发，找到
void Time_cb()
{

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


void Write_cb(APP *app , Reactor *R, APP::Server_Ptr server, ThreadPool &th_pool)
{
    int clientfd = R->Get_Now_Event().data.fd;
    APP::Tcp_Conn_Ptr conn = std::dynamic_pointer_cast<Tcp_Conn>(server->Get_Conn(clientfd));
    string res;
    while(!conn->Has_Finished_Future()){
    };
    res = conn->Get_An_Finish_Task();
    if(!res.empty()){
        conn->Appand_Wbuffer(res);
        server->Send(conn, conn->Get_Wbuffer_Length());
        conn->Erase_Wbuffer(conn->Get_Wbuffer_Length());
    }
    R->Mod_Reactor(clientfd,EPOLLIN);
}
/**
 * @description: 初始化应用
 * @return {*}
 * @author: Gong
 */

int APP::Init(uint16_t port,uint16_t backlog,uint8_t thread_num,uint32_t per_max_rcv_size)
{
    th_pool.Create(thread_num);
    Server_Ptr server = std::make_shared<Server>();
    R->Add_Server(server);
    server->Bind(port);
    server->Listen(backlog);
    R->Set_No_Block(server->Get_Sock());
    R->Add_Reactor(server->Get_Sock(),EPOLLIN);
    R->Set_Accept_cb(std::bind(Accept_cb, R, server));
    R->Set_Read_cb(std::bind(Read_cb, this, R, server, std::ref(th_pool)));
    R->Set_Write_cb(std::bind(Write_cb, this, R, server, std::ref(th_pool)));
    return 0;
}

/**
 * @description: 解析命令
 * @param {string} buffer
 * @return string
 * @author: Gong
 */
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
            //else if(i == CMD_EVENTBEG)
                //未实现
                
            break;
        }
    }
    if(i==CMD_END) ret_info=ret_msg[RET_CMD_ERROR];
    return ret_info;
}


string APP::Work(APP::Tcp_Conn_Ptr& conn)
{
    //解析协议 //此处应重新实现以防TCP粘包
    string buffer(conn->Get_Rbuffer()); 
    string res = Decode(buffer);
    conn->Erase_Rbuffer(conn->Get_Rbuffer_Length());
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
    std::unique_lock lock(*string_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string, std::shared_ptr<Security_String>>::iterator it = string_store.store.find(key);
    if(it == string_store.store.end()){
        it = string_store.store.emplace(key,std::make_shared<Security_String>()).first;
    }
    lock.unlock();
    std::unique_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    it->second->security_string = value;
    key_lock.unlock();
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_Get(string key)
{
    std::shared_lock lock(*string_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_String>>::iterator it=string_store.store.find(key);
    if(it==string_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::shared_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    return it->second->security_string+"\r\n";;
}

string APP::Exec_Cmd_Appand(string key, string value)
{
    std::shared_lock lock(*string_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_String>>::iterator it=string_store.store.find(key);
    if(it==string_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::unique_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    it->second->security_string.append(value);
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_Len(string key)
{
    std::shared_lock lock(*string_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_String>>::iterator it=string_store.store.find(key);
    if(it==string_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::shared_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    return std::to_string(it->second->security_string.length())+"\r\n";
}

string APP::Exec_Cmd_Delete(string key)
{
    std::unique_lock lock(*string_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_String>>::iterator it=string_store.store.find(key);
    if(it==string_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    string_store.store.erase(it);
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_Exist(string key)
{
    std::shared_lock lock(*string_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_String>>::iterator it=string_store.store.find(key);
    if(it==string_store.store.end()){
        return ret_msg[RET_NOT_EXIST];
    }
    return ret_msg[RET_EXIST];
}

string APP::Exec_Cmd_ASet(string key,vector<string_view> &res)
{
    std::unique_lock lock(*array_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_Array>>::iterator it = array_store.store.find(key);
    if(it == array_store.store.end()){
        it = array_store.store.emplace(key,std::make_shared<Security_Array>()).first;
    }
    lock.unlock();
    std::unique_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    for(int i=2;i < res.size();i++)
    {
        //这里不能使用empalce_back，匿名对象会被释放
        string temp(res[i]);
        (*it).second->security_array.emplace_back(temp);
    }
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_AGet(string key)
{
    std::shared_lock lock(*array_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_Array>>::iterator it = array_store.store.find(key);
    //未找到key
    if( it == array_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::shared_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    //使用span进行只读操作
    span<string> res((*it).second->security_array);
    string ret_value;
    for(int i=0;i<res.size();i++){
        ret_value = ret_value + res[i] + "\r\n";
    }
    return ret_value;
}

string APP::Exec_Cmd_ACount(string key)
{
    std::shared_lock lock(*array_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_Array>>::iterator it = array_store.store.find(key);
    //未找到key
    if( it == array_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::shared_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    return std::to_string((*it).second->security_array.size())+"\r\n";
}

string APP::Exec_Cmd_ADelete(string key,vector<string_view> &res)
{
    std::unique_lock lock(*array_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    string ret_value;
    std::map<std::string,std::shared_ptr<Security_Array>>::iterator it = array_store.store.find(key);
    //未找到key
    if( it == array_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    int count = 0;
    if(res.size()==2){
        count = it->second->security_array.size();
        array_store.store.erase(it);
    }else{
        lock.unlock();
        std::unique_lock key_lock(*it->second->mtx,std::defer_lock);
        while (!key_lock.try_lock())
        {
            spdlog::debug("lock failed\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
        }
        for (int i = 2; i < res.size(); i++)
        {
            for (vector<string>::iterator array_it = (*it).second->security_array.begin(); array_it != (*it).second->security_array.end(); array_it++)
            {
                if (res[i].compare(*array_it) == 0)
                {
                    array_it = (*it).second->security_array.erase(array_it);
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
    std::shared_lock lock(*array_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_Array>>::iterator it = array_store.store.find(key);
    if( it == array_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::shared_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    for (vector<string>::iterator array_it = (*it).second->security_array.begin(); array_it != (*it).second->security_array.end(); array_it++)
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
    std::unique_lock lock(*list_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    //使用迭代器防止减少每次从红黑树中查询的操作
    std::map<std::string,std::shared_ptr<Security_List>>::iterator it = list_store.store.find(key);
    if( it == list_store.store.end()){
        it = list_store.store.emplace(key,std::make_shared<Security_List>()).first;
    }
    lock.unlock();

    std::unique_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    for(int i=2;i<res.size();i++)
    {
        (*it).second->security_list.push_front(string(res[i]));
    }
    
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_RPUSH(string key, vector<string_view> &res)
{
    std::unique_lock lock(*list_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    //使用迭代器防止减少每次从红黑树中查询的操作
    std::map<std::string,std::shared_ptr<Security_List>>::iterator it = list_store.store.find(key);
    if( it == list_store.store.end()){
        it = list_store.store.emplace(key,std::make_shared<Security_List>()).first;
    }
    lock.unlock();
    std::unique_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    for(int i=2;i<res.size();i++)
    {
        (*it).second->security_list.push_back(string(res[i]));
    }
    
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_LGet(string key)
{
    std::shared_lock lock(*list_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_List>>::iterator it = list_store.store.find(key);
    //未找到key
    if( it == list_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::shared_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    string ret_value;
    for(list<string>::iterator list_it=(*it).second->security_list.begin();list_it!=(*it).second->security_list.end();list_it++)
    {
        ret_value=ret_value+(*list_it)+"\r\n";
    }
    return ret_value;
}

string APP::Exec_Cmd_LCount(string key)
{
    std::shared_lock lock(*list_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_List>>::iterator it = list_store.store.find(key);
    //未找到key
    if( it == list_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::shared_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    return std::to_string((*it).second->security_list.size())+"\r\n";
}

string APP::Exec_Cmd_LDelete(string key, vector<string_view> &res)
{
    std::unique_lock lock(*list_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_List>>::iterator it = list_store.store.find(key);
    //未找到key
    if( it == list_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    int count=0;
    if(res.size() == 2){
        count=(*it).second->security_list.size();
        list_store.store.erase(it);
    }else{
        lock.unlock();
        std::unique_lock key_lock(*it->second->mtx,std::defer_lock);
        while (!key_lock.try_lock())
        {
            spdlog::debug("lock failed\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
        }
        for (int i = 2; i < res.size(); i++)
        {
            for (list<string>::iterator list_it = (*it).second->security_list.begin(); list_it != (*it).second->security_list.end(); list_it++)
            {
                if ((*list_it).compare(res[i]) == 0)
                {
                    list_it = (*it).second->security_list.erase(list_it);
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
    std::shared_lock lock(*list_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_List>>::iterator it = list_store.store.find(key);
    if( it == list_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::shared_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    for (list<string>::iterator list_it = (*it).second->security_list.begin(); list_it != (*it).second->security_list.end(); list_it++)
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
    std::unique_lock lock(*rbtree_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    //使用迭代器防止减少每次从红黑树中查询的操作
    std::map<std::string,std::shared_ptr<Security_RBtree>>::iterator it = rbtree_store.store.find(key);
    if( it == rbtree_store.store.end()){
        it = rbtree_store.store.emplace(key,std::make_shared<Security_RBtree>()).first;
    }
    lock.unlock();
    std::unique_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    for(int i=2;i<res.size();i += 2){
        it->second->security_rbtree[string(res[i])] = string(res[i+1]);
    }
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_RGet(string key,string field)
{
    std::shared_lock lock(*rbtree_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_RBtree>>::iterator it=rbtree_store.store.find(key);
    if(it == rbtree_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::shared_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,string>::iterator rb_it=(*it).second->security_rbtree.find(field);
    if(rb_it == (*it).second->security_rbtree.end()){
        return ret_msg[RET_NO_FIELD];
    }
    return (*rb_it).second + "\r\n";
}

string APP::Exec_Cmd_RCount(string key)
{
    std::shared_lock lock(*rbtree_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_RBtree>>::iterator it=rbtree_store.store.find(key);
    if(it == rbtree_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::shared_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    return std::to_string((*it).second->security_rbtree.size())+"\r\n";
}

string APP::Exec_Cmd_RDelete(string key,vector<string_view> &res)
{
    std::unique_lock lock(*rbtree_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_RBtree>>::iterator it=rbtree_store.store.find(key);
    if(it == rbtree_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    int count=0;
    if(res.size()==2){
        count=(*it).second->security_rbtree.size();
        rbtree_store.store.erase(it);
    }else{
        lock.unlock();
        std::unique_lock key_lock(*it->second->mtx,std::defer_lock);
        while (!key_lock.try_lock())
        {
            spdlog::debug("lock failed\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
        }
        for(int i=2;i<res.size();i++)
        {
            if((*it).second->security_rbtree.find(string(res[i]))!=(*it).second->security_rbtree.end()){
                (*it).second->security_rbtree.erase(string(res[i]));
                count++;
            }
        }
    }
    return std::to_string(count)+"\r\n";
}

string APP::Exec_Cmd_RExist(string key, string field)
{
    std::shared_lock lock(*rbtree_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_RBtree>>::iterator it=rbtree_store.store.find(key);
    if(it == rbtree_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::shared_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    if((*it).second->security_rbtree.find(field)==(*it).second->security_rbtree.end()){
        return ret_msg[RET_NOT_EXIST];
    }
    return ret_msg[RET_EXIST];
}

string APP::Exec_Cmd_SSet(string key, vector<string_view> &res)
{
    std::unique_lock lock(*set_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_Set>>::iterator it = set_store.store.find(key);
    if( it == set_store.store.end()){
        it = set_store.store.emplace(key,std::make_shared<Security_Set>()).first;
    }
    lock.unlock();
    std::unique_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    for(int i=2;i<res.size();i++)
    {
        it->second->security_set.insert(string(res[i]));
    }
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_SGet(string key)
{
    std::shared_lock lock(*set_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_Set>>::iterator it = set_store.store.find(key);
    //未找到key
    if( it == set_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::shared_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    string ret_value;
    for(unordered_set<string>::iterator set_it = (*it).second->security_set.begin();set_it!=(*it).second->security_set.end();set_it++){
        ret_value=ret_value+(*set_it)+"\r\n";
    }
    return ret_value;
}

string APP::Exec_Cmd_SCount(string key)
{
    std::shared_lock lock(*set_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_Set>>::iterator it = set_store.store.find(key);
    //未找到key
    if( it == set_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::shared_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    return std::to_string((*it).second->security_set.size())+"\r\n";
}

string APP::Exec_Cmd_SDelete(string key, vector<string_view> &res)
{
    std::unique_lock lock(*set_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    string ret_value;
    std::map<std::string,std::shared_ptr<Security_Set>>::iterator it = set_store.store.find(key);
    //未找到key
    if( it == set_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    int count = 0;
    if(res.size()==2){
        count = (*it).second->security_set.size();
        set_store.store.erase(it);
    }else{
        lock.unlock();
        std::unique_lock key_lock(*it->second->mtx,std::defer_lock);
        while (!key_lock.try_lock())
        {
            spdlog::debug("lock failed\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
        }
        unordered_set<string>::iterator set_it;
        for (int i = 2; i < res.size(); i++)
        {
            set_it=(*it).second->security_set.find(string(res[i]));
            if(set_it!=(*it).second->security_set.end()){
                set_it = (*it).second->security_set.erase(set_it);
                count++;
            }
        }
    }
    return std::to_string(count)+"\r\n";
}

string APP::Exec_Cmd_SExist(string key, string value)
{
    std::shared_lock lock(*set_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_Set>>::iterator it = set_store.store.find(key);
    if( it == set_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    std::shared_lock key_lock(*it->second->mtx,std::defer_lock);
    while(!key_lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    unordered_set<string>::iterator set_it = (*it).second->security_set.find(value);
    if(set_it==(*it).second->security_set.end()){
        return ret_msg[RET_NOT_EXIST];
    }
    return ret_msg[RET_EXIST];
}
