#pragma once
#include <cstdint>
#ifndef DISPOSITION_HPP
#define DISPOSITION_HPP
#endif
#include<iostream>
#include<fstream>
#include"rapidjson/document.h"
using namespace rapidjson;
using std::ifstream;
#define DEFAULT_PORT 9999
#define DEFAUL_BUFFER_SIZE 1024
#define DEFAULT_EVENT_NUM 1024
#define DEFAULT_THREAD_NUM 1024
#define DEFAULT_BACKLOG 10
/*
{
    "port": 9999,
    "event_num":1024,
    "thread_num":4,
    "default_buffer_size": 2048
}
*/
class Disposition
{
public:
    Disposition(const char* disposition_file)
    {
        ifstream file;
        file.open(disposition_file);
        if(!file.is_open()){
            printf("open disposition file failed!\n");
            exit(0);
        }
        file.seekg(0,std::ios::end);
        uint64_t length = file.tellg();
        file.seekg(0,std::ios::beg);
        char *buffer=new char[length];
        memset(buffer,0,length);
        file.read(buffer,length);
        Document document;
        document.Parse(buffer);
        if(document.HasParseError()){
            printf("Prase Error!\n");
            file.close();
            exit(0);
        }
        file.close();
        Generate(document);
    }
    
private:
    void Generate(Document &document)
    {
        if(document.HasMember("port")){
            if(document["port"].IsInt()){
                _port = document["port"].GetInt();
            }else{
                printf("json file has error!\n");
                exit(0);
            }
        }else{
            _port = DEFAULT_PORT;
        }
        if(document.HasMember("event_num")){
            if(document["event_num"].IsInt()){
                _event_num = document["event_num"].GetInt();
            }else{
                printf("json file has error!\n");
                exit(0);
            }
        }else{
            _event_num = DEFAULT_EVENT_NUM;
        }
        if(document.HasMember("thread_num")){
            if(document["thread_num"].IsInt()){
                _thread_num = document["thread_num"].GetInt();
            }else{
                printf("json file has error!\n");
                exit(0);
            }
        }else{
            _thread_num = DEFAULT_THREAD_NUM;
        }
        if(document.HasMember("backlog")){
            if(document["backlog"].IsInt()){
                _backlog = document["backlog"].GetInt();
            }else{
                printf("json file has error!\n");
                exit(0);
            }
        }else{
            _backlog = DEFAULT_BACKLOG;
        }
        if(document.HasMember("default_buffer_size")){
            if(document["default_buffer_size"].IsInt()){
                _buffer_size = document["default_buffer_size"].GetInt();
            }else{
                printf("json file has error!\n");
                exit(0);
            }
        }else{
            _buffer_size = DEFAUL_BUFFER_SIZE;
        }
    }

public:
    
    uint16_t _port;
    uint32_t _event_num;
    uint32_t _thread_num;
    uint16_t _backlog;
    uint32_t _buffer_size;
};