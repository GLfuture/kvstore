/*
 * @Description:
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-10-01 06:10:54
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-05 05:18:39
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