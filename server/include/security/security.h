/*
 * @Description:
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-10-01 06:10:54
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-11 13:20:55
 */

#pragma once
#include <string>
#include <future>
#include <vector>
#include <list>
#include <map>
#include <unordered_set>
#include <memory>
#include <atomic>
#include <condition_variable>
#include <shared_mutex>
#include <queue>
#include <stack>


class Security_Future_Queue
{
public:
    Security_Future_Queue(){
        mtx = std::make_shared<std::mutex>();
    }
    Security_Future_Queue(const Security_Future_Queue&& other){
        this->mtx = std::move(other.mtx);
        this->seurity_queue = std::move(other.seurity_queue);
    }
    Security_Future_Queue(const Security_Future_Queue*& other){
        this->mtx = std::move(other->mtx);
        this->seurity_queue = std::move(other->seurity_queue);
    }
    std::shared_ptr<std::mutex> mtx;
    std::queue<std::shared_future<std::string>> seurity_queue;
};

class Security_Cmd_Queue
{
public:
    Security_Cmd_Queue(){
        mtx = std::make_shared<std::mutex>();
    }
    Security_Cmd_Queue(const Security_Cmd_Queue&& other){
        this->mtx = std::move(other.mtx);
        this->seurity_queue = std::move(other.seurity_queue);
    }
    Security_Cmd_Queue(const Security_Cmd_Queue*& other){
        this->mtx = std::move(other->mtx);
        this->seurity_queue = std::move(other->seurity_queue);
    }
    std::shared_ptr<std::mutex> mtx;
    std::queue<std::string> seurity_queue;
};

class Security_Cmd_Vector
{
public:
    Security_Cmd_Vector(){
        mtx = std::make_shared<std::mutex>();
    }
    Security_Cmd_Vector(const Security_Cmd_Vector&& other){
        this->mtx = std::move(other.mtx);
        this->security_vector = std::move(other.security_vector);
    }
    Security_Cmd_Vector(const Security_Cmd_Vector*& other){
        this->mtx = std::move(other->mtx);
        this->security_vector = std::move(other->security_vector);
    }
    std::shared_ptr<std::mutex> mtx;
    std::vector<std::string> security_vector;
};

class Security_Base
{
public:
    Security_Base()
    {
        mtx = std::make_shared<std::shared_mutex>();
    }
    std::shared_ptr<std::shared_mutex> mtx;
};

class Security_String : public Security_Base
{
public:
    Security_String() : Security_Base()
    {
        this->security_string ="";
    }

    Security_String(const Security_String &&other)
    {
        this->mtx = std::move(other.mtx);
        this->security_string = std::move(other.security_string);
    }
    Security_String(const Security_String *&other)
    {
        this->mtx = std::move(other->mtx);
        this->security_string = std::move(other->security_string);
    }
    std::string security_string;
};

class Security_Array : public Security_Base
{
public:
    Security_Array() : Security_Base()
    {
    }
    Security_Array(const Security_Array &&other)
    {

        this->mtx = std::move(other.mtx);
        this->security_array = std::move(other.security_array);
    }
    Security_Array(const Security_Array *&other)
    {

        this->mtx = std::move(other->mtx);
        this->security_array = std::move(other->security_array);
    }
    std::vector<std::string> security_array;
};

class Security_List : public Security_Base
{
public:
    Security_List() : Security_Base()
    {
    }
    Security_List(const Security_List &&other)
    {

        this->mtx = std::move(other.mtx);
        this->security_list = std::move(other.security_list);
    }
    Security_List(const Security_List *&other)
    {

        this->mtx = std::move(other->mtx);
        this->security_list = std::move(other->security_list);
    }
    std::list<std::string> security_list;
};

class Security_RBtree : public Security_Base
{
public:
    Security_RBtree() : Security_Base()
    {
    }
    Security_RBtree(const Security_RBtree &&other)
    {

        this->mtx = std::move(other.mtx);
        this->security_rbtree = std::move(other.security_rbtree);
    }
    Security_RBtree(const Security_RBtree *&other)
    {

        this->mtx = std::move(other->mtx);
        this->security_rbtree = std::move(other->security_rbtree);
    }
    std::map<std::string, std::string> security_rbtree;
};

class Security_Satck : public Security_Base
{
public:
    Security_Satck() : Security_Base()
    {
    }
    Security_Satck(const Security_Satck &&other)
    {

        this->mtx = std::move(other.mtx);
        this->security_stack = std::move(other.security_stack);
    }
    Security_Satck(const Security_Satck *&other)
    {

        this->mtx = std::move(other->mtx);
        this->security_stack = std::move(other->security_stack);
    }
    std::stack<std::string> security_stack;
};



class Security_Set : public Security_Base
{
public:
    Security_Set() : Security_Base()
    {
    }
    Security_Set(const Security_Set &&other)
    {

        this->mtx = std::move(other.mtx);
        this->security_set = std::move(other.security_set);
    }
    Security_Set(const Security_Set *&other)
    {

        this->mtx = std::move(other->mtx);
        this->security_set = std::move(other->security_set);
    }
    std::unordered_set<std::string> security_set;
};

class Map_Security_String_Store : public Security_Base
{
public:
    Map_Security_String_Store() : Security_Base()
    {
    }
    Map_Security_String_Store(const Map_Security_String_Store &&other)
    {

        this->mtx = std::move(other.mtx);
        this->store = std::move(other.store);
    }
    std::map<std::string, std::shared_ptr<Security_String>> store;
};

class Map_Security_Array_Store : public Security_Base
{
public:
    Map_Security_Array_Store() : Security_Base()
    {
    }
    Map_Security_Array_Store(const Map_Security_Array_Store &&other)
    {

        this->mtx = std::move(other.mtx);
        this->store = std::move(other.store);
    }
    std::map<std::string, std::shared_ptr<Security_Array>> store;
};

class Map_Security_List_Store : public Security_Base
{
public:
    Map_Security_List_Store() : Security_Base()
    {
    }
    Map_Security_List_Store(const Map_Security_List_Store &&other)
    {

        this->mtx = std::move(other.mtx);
        this->store = std::move(other.store);
    }
    std::map<std::string, std::shared_ptr<Security_List>> store;
};

class Map_Security_RBtree_Store : public Security_Base
{
public:
    Map_Security_RBtree_Store() : Security_Base()
    {
    }
    Map_Security_RBtree_Store(const Map_Security_RBtree_Store &&other)
    {

        this->mtx = std::move(other.mtx);
        this->store = std::move(other.store);
    }
    std::map<std::string, std::shared_ptr<Security_RBtree>> store;
};

class Map_Security_Set_Store : public Security_Base
{
public:
    Map_Security_Set_Store() : Security_Base()
    {
    }
    Map_Security_Set_Store(const Map_Security_Set_Store &&other)
    {
        this->mtx = std::move(other.mtx);
        this->store = std::move(other.store);
    }
    std::map<std::string, std::shared_ptr<Security_Set>> store;
};

// class Security_Back_String : public Security_String
// {
// public:
//     Security_Back_String() : Security_String()
//     {
//     }
//     Security_Back_String(const Security_Back_String &&other)
//     {
//         this->last_string = std::move(other.last_string);
//         this->mtx = std::move(other.mtx);
//         this->security_string = std::move(other.security_string);
//     }
//     Security_Back_String(const Security_Back_String *&other)
//     {
//         this->last_string = std::move(other->last_string);
//         this->mtx = std::move(other->mtx);
//         this->security_string = std::move(other->security_string);
//     }
//     std::shared_ptr<Security_Satck> last_string;
// };

// class Security_Back_RBtree :public Security_RBtree
// {
// public:
//     Security_Back_RBtree() : Security_RBtree()
//     {
//     }
//     Security_Back_RBtree(const Security_Back_RBtree &&other)
//     {
//         this->mtx = std::move(other.mtx);
//         this->security_rbtree = std::move(other.security_rbtree);
//     }
//     Security_Back_RBtree(const Security_Back_RBtree *&other)
//     {
//         this->mtx = std::move(other->mtx);
//         this->security_rbtree = std::move(other->security_rbtree);
//     }
//     std::shared_ptr<std::map<std::string,Security_Satck>> last_field_string;
// };


class Security_Back_Store : public Security_Base
{
public:
    Security_Back_Store() : Security_Base()
    {
    }
    Security_Back_Store(const Security_Back_Store &&other)
    {
        this->mtx = std::move(other.mtx);
        this->string_store = std::move(other.string_store);
        this->array_store = std::move(other.array_store);
        this->list_store = std::move(other.list_store);
        this->rbtree_store = std::move(other.rbtree_store);
        this->set_store = std::move(other.set_store);
    }

    
    std::map<std::string, std::shared_ptr<Security_String>> string_store;
    std::map<std::string, std::shared_ptr<Security_Array>> array_store;
    std::map<std::string, std::shared_ptr<Security_List>> list_store;
    std::map<std::string, std::shared_ptr<Security_RBtree>> rbtree_store;
    std::map<std::string, std::shared_ptr<Security_Set>> set_store;

};


class Security_Recorder
{
public:
    Security_Recorder()
    {
        num.store(0 , std::memory_order::relaxed);
        Is_Record.store(false , std::memory_order::relaxed);
    }
    
    void Add_Num()
    {
        num.fetch_add(1,std::memory_order::relaxed);
    }

    int Get_Num()
    {
        return num.load();
    }
    
    void Reset()
    {
        num.store(0,std::memory_order::relaxed);
    }

    void Set_Is_Record(bool is_record)
    {
        Is_Record.store(is_record,std::memory_order::relaxed);
    }

    bool Get_Is_Record()
    {
        return Is_Record.load();
    }

    std::atomic_char32_t num;
    std::atomic_bool Is_Record;
};