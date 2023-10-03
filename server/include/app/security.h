/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-10-01 06:10:54
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-03 07:39:12
 */

#pragma once
#include <mutex>
#include <string>
#include <future>
#include <vector>
#include <list>
#include <map>
#include <unordered_set>
#include <memory>



class Security_String
{
public:
    Security_String(){
        mtx = std::make_shared<std::mutex>();
    }
    Security_String(const Security_String&& other){
        this->mtx = std::move(other.mtx);
        this->security_string = std::move(other.security_string);
    }
    Security_String(const Security_String*& other){
        this->mtx = std::move(other->mtx);
        this->security_string = std::move(other->security_string);
    }
    std::string security_string;
    std::shared_ptr<std::mutex> mtx;
};

class Security_Array
{
public:
    Security_Array(){
        mtx = std::make_shared<std::mutex>();
    }
    Security_Array(const Security_Array&& other){
        this->mtx = std::move(other.mtx);
        this->security_array = std::move(other.security_array);
    }
    Security_Array(const Security_Array*& other){
        this->mtx = std::move(other->mtx);
        this->security_array = std::move(other->security_array);
    }
    std::shared_ptr<std::mutex> mtx;
    std::vector<std::string> security_array;
};

class Security_List
{
public:
    Security_List(){
        mtx = std::make_shared<std::mutex>();
    }
    Security_List(const Security_List&& other){
        this->mtx = std::move(other.mtx);
        this->security_list = std::move(other.security_list);
    }
    Security_List(const Security_List*& other){
        this->mtx = std::move(other->mtx);
        this->security_list = std::move(other->security_list);
    }
    std::shared_ptr<std::mutex> mtx;
    std::list<std::string> security_list;
};

class Security_RBtree
{
public:
    Security_RBtree(){
        mtx = std::make_shared<std::mutex>();
    }
    Security_RBtree(const Security_RBtree&& other){
        this->mtx = std::move(other.mtx);
        this->security_rbtree = std::move(other.security_rbtree);
    }
    Security_RBtree(const Security_RBtree*& other){
        this->mtx = std::move(other->mtx);
        this->security_rbtree = std::move(other->security_rbtree);
    }
    std::shared_ptr<std::mutex> mtx;
    std::map<std::string,std::string> security_rbtree;
};

class Security_Set
{
public:
    Security_Set(){
        mtx = std::make_shared<std::mutex>();
    }
    Security_Set(const Security_Set&& other){
        this->mtx = std::move(other.mtx);
        this->security_set = std::move(other.security_set);
    }
    Security_Set(const Security_Set*& other){
        this->mtx = std::move(other->mtx);
        this->security_set = std::move(other->security_set);
    }
    std::shared_ptr<std::mutex> mtx;
    std::unordered_set<std::string> security_set;
};