#include "app/app.h"
#include <cstdint>

/**
 * @description: 初始化应用
 * @return {*}
 * @author: Gong
 */

int APP::Init_cb(std::function<void()> &accept_cb, std::function<void()> &read_cb, std::function<void()> &write_cb
    , std::function<void()> &exit_cb)
{
    R->Set_Accept_cb(std::move(accept_cb));
    R->Set_Read_cb(std::move(read_cb));
    R->Set_Write_cb(std::move(write_cb));
    R->Set_Exit_cb(std::move(exit_cb));
    return 0;
}

/**
 * @description: 解析命令
 * @param {string} buffer
 * @return string
 * @author: Gong
 */
string APP::Decode(Msg& msg)
{
    //std::cout<<msg.buffer<<std::endl;
    Split split(msg.buffer);
    if(!split.Spilt_With_Char_Connect_With_Quote(' ')){
        return ret_msg[RET_STR_ERROR];
    }
    vector<string_view> res = split.Get_Result();
    string ret_info = ret_msg[RET_NULL];
    int i;
    for(i = CMD_BEG ; i < CMD_END ; i++)
    {
        if(res[0].compare(cmds[CMD_EVENTEND]) == 0)
        {
            ret_info = Exec_Cmd_Event_End(msg);
            break;
        }
        if(msg.Event_Start->load()){
            affairs[msg.fd]->Ready_Add_Cmd(msg.buffer);
            break;
        }
        if(res[0].compare(cmds[i]) == 0){
            if(!msg.Event_Start->load()){
                if (i <= CMD_EXIST)
                    ret_info = Decode_String(msg,i, res);
                else if (i <= CMD_AEXIST)
                    ret_info = Decode_Array(msg,i, res);
                else if (i <= CMD_LEXIST)
                    ret_info = Decode_List(msg,i, res);
                else if (i <= CMD_REXIST)
                    ret_info = Decode_Rbtree(msg,i, res);
                else if (i <= CMD_SEXIST)
                    ret_info = Decode_Set(msg,i, res);
                else if(i == CMD_EVENTBEG)
                    ret_info = Exec_Cmd_Eevent_Beg(msg);
                else if(i == CMD_ROLLBACK)
                    ret_info = Exec_Cmd_RollBack(msg);
                else if(i == CMD_CLEAN_CACHE)
                    ret_info = Exec_Cmd_Clean_Cache(msg);
                else if(i == CMD_CLEAN_AOF)
                    ret_info = Exec_Cmd_Clean_AOF(msg);
            }
            break;
        }
    }
    if(i == CMD_END) ret_info=ret_msg[RET_CMD_ERROR];
    //std::cout<<ret_info<<" len :"<<ret_info.length()<<std::endl;
    return ret_info;
}

int APP::Read_AOF_And_Init()
{
    uintmax_t size = 0;
    if(file_io->exist(file_io->get_filename()))
    {
        size = file_io->size(file_io->get_filename());
    }else{
        spdlog::info("{}\n",strerror(errno));
    }
    std::string buffer ="";
    file_io->Is_Loading = true;
    if(size < LIMIT_STREAM_SIZE)
    {
        if(file_io->open(File_IO::STREAM,file_io->get_filename()) != 0){
            spdlog::error("{}\n",strerror(errno));
            return -1;
        }
        buffer.assign(file_io->read(File_IO::STREAM,size),size);
    }else{
        if(file_io->open(File_IO::MMAP,file_io->get_filename()) != 0){
            spdlog::error("{}\n",strerror(errno));
            return -1;
        }
        buffer.assign(file_io->read(File_IO::MMAP,size),size);
    }
    //std::cout<<buffer<<std::endl;
    Split spilt(buffer);
    spilt.Spilt_With_Char('\n');
    std::vector<std::string_view> res = spilt.Get_Result();
    for(int i=0;i<res.size();i++)
    {
        Msg msg;
        msg.buffer = std::string(res[i]);
        // std::cout<<msg.buffer<<std::endl;
        Decode(msg);
    
    }
    file_io->Is_Loading = false;

    //不需要删除
    //std::remove(file_io->Get_File_Name().c_str());
    return 0;
}

void APP::Deal_Closed_Conn(Tcp_Conn_Ptr &conn_ptr,uint32_t event)
{
    R->Del_Reactor(conn_ptr->Get_Conn_fd(), event);
    Server_Ptr server = std::dynamic_pointer_cast<Server>(R->Get_Server());
    server->Finish_All_Future_Before_Del_Conn(conn_ptr);
    affairs.erase(conn_ptr->Get_Conn_fd());
}


Reactor::Timer_Ptr APP::Add_Time_Out_cb(uint16_t timerid, uint64_t interval_time, Timer::TimerType type, function<void()> &&timeout_cb) 
{
    return R->Set_Timeout_cb(timerid,interval_time, type,std::move(timeout_cb));
}

string APP::Work(APP::Tcp_Conn_Ptr& conn_ptr)
{
    string res="";
    Msg msg(conn_ptr->Get_Conn_fd(),string(conn_ptr->Get_Rbuffer()),conn_ptr->Get_Affairs_Status());
    res = Decode(msg);
    conn_ptr->Erase_Rbuffer(conn_ptr->Get_Rbuffer_Length());
    return res;
}

string APP::Decode_String(Msg& msg,int cmd_type, vector<string_view> &res)
{
    string ret_info;
    switch (cmd_type)
    {
        case CMD_SET:
        {
            if(res.size() != 3)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_Set(msg,string(res[1]),string(res[2]));
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
            ret_info = Exec_Cmd_Appand(msg,string(res[1]),string(res[2]));
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
            ret_info = Exec_Cmd_Delete(msg,string(res[1]));
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

string APP::Decode_Array(Msg& msg,int cmd_type, vector<string_view> &res)
{
    string ret_info;
    switch (cmd_type)
    {
        case CMD_ASET:
        {
            if(res.size()<3)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_ASet(msg,string(res[1]),res);
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
            ret_info = Exec_Cmd_ADelete(msg,string(res[1]),res);
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

string APP::Decode_List(Msg& msg,int cmd_type, vector<string_view> &res)
{
    string ret_info;
    switch (cmd_type)
    {
        case CMD_LPUSH:
        {
            if(res.size()<3)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_LPUSH(msg,string(res[1]),res);
            return ret_info;
        }
        case CMD_RPUSH:
        {
            if(res.size()<3)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_RPUSH(msg,string(res[1]),res);
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
            ret_info = Exec_Cmd_LDelete(msg,string(res[1]),res);
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

string APP::Decode_Rbtree(Msg& msg,int cmd_type, vector<string_view> &res)
{
    string ret_info;
    switch (cmd_type)
    {
        case CMD_RSET:
        {
            if (res.size() < 4 || res.size() % 2 != 0)
            return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_RSet(msg,string(res[1]), res);
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
            ret_info = Exec_Cmd_RDelete(msg,string(res[1]),res);
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

string APP::Decode_Set(Msg& msg,int cmd_type, vector<string_view> &res)
{
    string ret_info;
    switch (cmd_type)
    {
        case CMD_SSET:
        {
            if(res.size()<3)   return ret_msg[RET_ARG_ERROR];
            ret_info = Exec_Cmd_SSet(msg,string(res[1]),res);
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
            ret_info = Exec_Cmd_SDelete(msg,string(res[1]),res);
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

string APP::Exec_Cmd_Set(Msg& msg, string key, string value)
{
    std::shared_lock all_lock(*back_store.mtx);
    std::unique_lock lock(*string_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string, std::shared_ptr<Security_String>>::iterator it = string_store.store.find(key);
    if(it == string_store.store.end()){
        it = string_store.store.emplace(key,std::make_shared<Security_String>()).first;
    }
    if(!file_io->Is_Loading){
        file_io->write(std::move(msg.buffer+"\n"));
        if(cmd_recorder.Get_Is_Record()) cmd_recorder.Add_Num();
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
    return it->second->security_string;
}

string APP::Exec_Cmd_Appand(Msg&msg ,string key, string value)
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
    if(!file_io->Is_Loading){
        file_io->write(std::move(msg.buffer+"\n"));
        if(cmd_recorder.Get_Is_Record()) cmd_recorder.Add_Num();
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
    return std::to_string(it->second->security_string.length());
}

string APP::Exec_Cmd_Delete(Msg& msg,string key)
{
    std::shared_lock all_lock(*back_store.mtx);
    std::unique_lock lock(*string_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    if(!file_io->Is_Loading){
        file_io->write(std::move(msg.buffer+"\n"));
        if(cmd_recorder.Get_Is_Record()) cmd_recorder.Add_Num();
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

string APP::Exec_Cmd_ASet(Msg& msg,string key,vector<string_view> &res)
{
    std::shared_lock all_lock(*back_store.mtx);
    std::unique_lock lock(*array_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_Array>>::iterator it = array_store.store.find(key);
    if(it == array_store.store.end()){
        it = array_store.store.emplace(key,std::make_shared<Security_Array>()).first;
    }
    if(!file_io->Is_Loading){
        file_io->write(std::move(msg.buffer+"\n"));
        if(cmd_recorder.Get_Is_Record()) cmd_recorder.Add_Num();
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
        ret_value = ret_value + res[i];
        if(i != res.size()-1) ret_value += "\n";
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
    return std::to_string((*it).second->security_array.size());
}

string APP::Exec_Cmd_ADelete(Msg& msg,string key,vector<string_view> &res)
{
    std::shared_lock all_lock(*back_store.mtx);
    std::unique_lock lock(*array_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    if(!file_io->Is_Loading){
        file_io->write(std::move(msg.buffer+"\n"));
        if(cmd_recorder.Get_Is_Record()) cmd_recorder.Add_Num();
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
        if(it->second->security_array.empty()){
        lock.lock();
        it = array_store.store.erase(it);
        lock.unlock();
    }
    }
    return std::to_string(count);
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

string APP::Exec_Cmd_LPUSH(Msg& msg,string key, vector<string_view> &res)
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
    if(!file_io->Is_Loading){
        file_io->write(std::move(msg.buffer+"\n"));
        if(cmd_recorder.Get_Is_Record()) cmd_recorder.Add_Num();
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

string APP::Exec_Cmd_RPUSH(Msg& msg,string key, vector<string_view> &res)
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
    if(!file_io->Is_Loading){
        file_io->write(std::move(msg.buffer+"\n"));
        if(cmd_recorder.Get_Is_Record()) cmd_recorder.Add_Num();
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
    int i=0;
    for(list<string>::iterator list_it=(*it).second->security_list.begin();list_it!=(*it).second->security_list.end();list_it++)
    {
        ret_value=ret_value+(*list_it);
        if( i < (*it).second->security_list.size()-1) ret_value+="\n";
        i++;
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
    return std::to_string((*it).second->security_list.size());
}

string APP::Exec_Cmd_LDelete(Msg& msg,string key, vector<string_view> &res)
{
    std::shared_lock all_lock(*back_store.mtx);
    std::unique_lock lock(*list_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    if(!file_io->Is_Loading){
        file_io->write(std::move(msg.buffer+"\n"));
        if(cmd_recorder.Get_Is_Record()) cmd_recorder.Add_Num();
    }
    std::map<std::string,std::shared_ptr<Security_List>>::iterator it = list_store.store.find(key);
    //未找到key
    if( it == list_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    int count=0;
    if(res.size() == 2){
        count = (*it).second->security_list.size();
        list_store.store.erase(it);
    }else{
        lock.unlock();
        std::unique_lock key_lock(*it->second->mtx, std::defer_lock);
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
        if (it->second->security_list.empty())
        {
            lock.lock();
            it = list_store.store.erase(it);
            lock.unlock();
        }
    }
    return std::to_string(count);
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

string APP::Exec_Cmd_RSet(Msg& msg,string key,vector<string_view> &res)
{
    std::shared_lock all_lock(*back_store.mtx);
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
    if(!file_io->Is_Loading){
        file_io->write(std::move(msg.buffer+"\n"));
        if(cmd_recorder.Get_Is_Record()) cmd_recorder.Add_Num();
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
    return (*rb_it).second;
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
    return std::to_string((*it).second->security_rbtree.size());
}

string APP::Exec_Cmd_RDelete(Msg& msg,string key,vector<string_view> &res)
{
    std::shared_lock all_lock(*back_store.mtx);
    std::unique_lock lock(*rbtree_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    if(!file_io->Is_Loading){
        file_io->write(std::move(msg.buffer+"\n"));
        if(cmd_recorder.Get_Is_Record()) cmd_recorder.Add_Num();
    }
    std::map<std::string,std::shared_ptr<Security_RBtree>>::iterator it=rbtree_store.store.find(key);
    if(it == rbtree_store.store.end()){
        return ret_msg[RET_NO_KEY];
    }
    int count=0;
    if(res.size()==2){
        count = (*it).second->security_rbtree.size();
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
        if (it->second->security_rbtree.empty())
        {
            lock.lock();
            it = rbtree_store.store.erase(it);
            lock.unlock();
        }
    }
    return std::to_string(count);
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

string APP::Exec_Cmd_SSet(Msg& msg,string key, vector<string_view> &res)
{
    std::shared_lock all_lock(*back_store.mtx);
    std::unique_lock lock(*set_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    std::map<std::string,std::shared_ptr<Security_Set>>::iterator it = set_store.store.find(key);
    if( it == set_store.store.end()){
        it = set_store.store.emplace(key,std::make_shared<Security_Set>()).first;
    }
    if(!file_io->Is_Loading){
        file_io->write(std::move(msg.buffer+"\n"));
        if(cmd_recorder.Get_Is_Record()) cmd_recorder.Add_Num();
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
    int i=0;
    for(unordered_set<string>::iterator set_it = (*it).second->security_set.begin();set_it!=(*it).second->security_set.end();set_it++){
        ret_value=ret_value+(*set_it);
        if(i < (*it).second->security_set.size()-1) ret_value+="\n";
        i++;
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
    return std::to_string((*it).second->security_set.size());
}

string APP::Exec_Cmd_SDelete(Msg& msg,string key, vector<string_view> &res)
{
    std::shared_lock all_lock(*back_store.mtx);
    std::unique_lock lock(*set_store.mtx,std::defer_lock);
    while(!lock.try_lock()){
        spdlog::debug("lock failed\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(this->wait_lock_millisec));
    }
    if(!file_io->Is_Loading){
        file_io->write(std::move(msg.buffer+"\n"));
        if(cmd_recorder.Get_Is_Record()) cmd_recorder.Add_Num();
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
        if (it->second->security_set.empty())
        {
            lock.lock();
            it = set_store.store.erase(it);
            lock.unlock();
        }
    }
    return std::to_string(count);
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

string APP::Exec_Cmd_Eevent_Beg(Msg& msg)
{
    Exec_Cmd_Clean_Cache(msg);
    BackUp();
    cmd_recorder.Set_Is_Record(true);
    msg.Event_Start->store(true);
    affairs.emplace(msg.fd,std::make_shared<Affairs>());
    return ret_msg[RET_NULL];
}

void APP::BackUp()
{
    std::unique_lock lock(*back_store.mtx);
    for(auto it = string_store.store.begin();it != string_store.store.end();it++)
    {
        //std::cout << it->first <<"  " <<it->second->security_string << std::endl;
        // std::cout << "size :" << string_store.store.size() << std::endl;
        // std::cout << "size :" << back_store.string_store.size() << std::endl;
        const auto& pair = back_store.string_store.emplace(it->first,std::make_shared<Security_String>());
        //std::cout << it->first <<"  " <<it->second->security_string << std::endl;
        pair.first->second->security_string = it->second->security_string;
        //std::cout<< pair.first->second->security_string <<std::endl;
    }

    for(auto it = array_store.store.begin();it!=array_store.store.end();it++)
    {
        const auto&pair =  back_store.array_store.emplace(it->first,std::make_shared<Security_Array>());
        pair.first->second->security_array.assign(it->second->security_array.begin(),it->second->security_array.end());
    }

    for(auto it = list_store.store.begin();it!=list_store.store.end();it++)
    {
        const auto&pair = back_store.list_store.emplace(it->first,std::make_shared<Security_List>());
        pair.first->second->security_list.assign(it->second->security_list.begin(),it->second->security_list.end());
    }

    for(auto it = rbtree_store.store.begin();it!=rbtree_store.store.end();it++)
    {
        const auto& pair = back_store.rbtree_store.emplace(it->first,std::make_shared<Security_RBtree>());
        for(auto mit = it->second->security_rbtree.begin();mit!=it->second->security_rbtree.end();mit++)
        {
            pair.first->second->security_rbtree[mit->first] = mit->second;
        }
    }

    for(auto it = set_store.store.begin();it!=set_store.store.end();it++)
    {
        const auto& pair = back_store.set_store.emplace(it->first,std::make_shared<Security_Set>());
        pair.first->second->security_set = it->second->security_set;
    }
}

string APP::Exec_Cmd_Event_End(Msg& msg)
{
    //防止死锁
    //std::unique_lock lock(*back_store.mtx);
    msg.Event_Start->store(false);
    std::map<uint32_t,Affair_Ptr>::iterator it = affairs.find(msg.fd);
    while(!it->second->ready_cmds->seurity_queue.empty()){
        std::string cmd = it->second->Get_An_Cmd();
        //std::cout<<"cmd : "<<cmd<<std::endl;
        Msg temp_msg(msg.fd,cmd,msg.Event_Start);
        std::string ret = Decode(temp_msg);
        if(ret == ret_msg[RET_STR_ERROR] || ret==ret_msg[RET_ARG_ERROR] || ret==ret_msg[RET_CMD_ERROR] 
            || ret==ret_msg[RET_NO_KEY] || ret== ret_msg[RET_NO_FIELD]){
            Exec_Cmd_RollBack(msg);
            affairs.erase(it);
            //std::cout << std::string(cmd + " : ") + ret <<std::endl;
            return std::string(cmd + " : ") + ret;
        }
    }
    affairs.erase(it);
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_RollBack(Msg &msg)
{
    std::unique_lock lock(*back_store.mtx);
    string_store.store.clear();
    array_store.store.clear();
    list_store.store.clear();
    rbtree_store.store.clear();
    set_store.store.clear();
    for(auto it = back_store.string_store.begin();it != back_store.string_store.end();it++)
    {
        //std::cout << it->first <<"  " <<it->second->security_string << std::endl;
        const auto& pair = string_store.store.emplace(it->first,std::make_shared<Security_String>());
        pair.first->second->security_string = it->second->security_string;
        //std::cout<<pair.first->second->security_string<<std::endl;
    }

    for(auto it = back_store.array_store.begin();it != back_store.array_store.end();it++)
    {
        const auto&pair =  array_store.store.emplace(it->first,std::make_shared<Security_Array>());
        pair.first->second->security_array.assign(it->second->security_array.begin(),it->second->security_array.end());
    }

    for(auto it = back_store.list_store.begin();it!=back_store.list_store.end();it++)
    {
        const auto&pair = list_store.store.emplace(it->first,std::make_shared<Security_List>());
        pair.first->second->security_list.assign(it->second->security_list.begin(),it->second->security_list.end());
    }

    for(auto it = back_store.rbtree_store.begin();it!=back_store.rbtree_store.end();it++)
    {
        const auto& pair = rbtree_store.store.emplace(it->first,std::make_shared<Security_RBtree>());
        for(auto mit = it->second->security_rbtree.begin();mit!=it->second->security_rbtree.end();mit++)
        {
            pair.first->second->security_rbtree[mit->first] = mit->second;
        }
    }

    for(auto it = back_store.set_store.begin();it!=back_store.set_store.end();it++)
    {
        const auto& pair = set_store.store.emplace(it->first,std::make_shared<Security_Set>());
        pair.first->second->security_set = it->second->security_set;
    }

    cmd_recorder.Set_Is_Record(false);

    file_io->close(File_IO::STREAM);

    file_io->erase_from_end(cmd_recorder.Get_Num());
    file_io->open(File_IO::STREAM,file_io->get_filename());
    cmd_recorder.Reset();
    return ret_msg[RET_OK];
}


string APP::Exec_Cmd_Clean_Cache(Msg& msg)
{
    std::shared_lock lock(*back_store.mtx);
    back_store.array_store.clear();
    back_store.list_store.clear();
    back_store.rbtree_store.clear();
    back_store.set_store.clear();
    back_store.string_store.clear();
    return ret_msg[RET_OK];
}

string APP::Exec_Cmd_Clean_AOF(Msg& msg)
{
    if(std::filesystem::exists(file_io->back_filename)){
        std::filesystem::remove(file_io->back_filename);
    }
    file_io->close(File_IO::STREAM);
    std::filesystem::rename(file_io->get_filename(),file_io->back_filename);
    file_io->open(File_IO::STREAM,file_io->get_filename());
    return ret_msg[RET_OK];
}