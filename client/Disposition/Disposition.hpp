#pragma once
#ifndef DISPOSITION_HPP
#define DISPOSITION_HPP
#endif
#include<iostream>
#include<fstream>
#include"rapidjson/document.h"
using namespace rapidjson;
using std::ifstream;
#define DEFAULT_REMOTE_ADDR "127.0.0.1"
#define DEFAULT_REMOTE_PORT 9898
#define DEFAUL_BUFFER_SIZE 1024
#define DEFAULT_RECV_LENGTH 0
#define DEFALUT_SEND_LENGTH 0

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
        if(document.HasMember("remote_addr")){
            if (document["remote_addr"].IsString()){
                _remote_addr = document["remote_addr"].GetString();
            }else{
                printf("json file has error!\n");
                exit(0);
            }
        }else{
            _remote_addr = DEFAULT_REMOTE_ADDR;
        }
        if(document.HasMember("remote_port")){
            if(document["remote_port"].IsInt()){
                _remote_port = document["remote_port"].GetInt();
            }else{
                printf("json file has error!\n");
                exit(0);
            }
        }else{
            _remote_port = DEFAULT_REMOTE_PORT;
        }
        if(document.HasMember("default_per_recv_length")){
            if(document["default_per_recv_length"].IsInt()){
                _recv_length = document["default_per_recv_length"].GetInt();
            }else if(document["default_per_recv_length"].IsString()&&document["default_per_recv_length"].GetString()=="default"){
                _recv_length = DEFAULT_RECV_LENGTH;
            }else{
                printf("json file has error!\n");
                exit(0);
            }
        }else{
            _recv_length = DEFAULT_RECV_LENGTH;
        }
        if(document.HasMember("default_per_send_length")){
            if(document["default_per_send_length"].IsInt()){
                _send_length = document["default_per_send_length"].GetInt();
            }else if(document["default_per_send_length"].IsString() && document["default_per_send_length"].GetString()==std::string("default")){
                _send_length = DEFALUT_SEND_LENGTH;
            }else{
                printf("json file has error!\n");
                exit(0);
            }
        }else{
            _send_length = DEFALUT_SEND_LENGTH;
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
    std::string _remote_addr;
    uint16_t _remote_port;
    uint32_t _recv_length;
    uint32_t _send_length;
    uint32_t _buffer_size;
};