/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-10-09 11:32:25
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-11 13:15:10
 */
#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <filesystem>
#include <sys/stat.h>
#include <spdlog/spdlog.h>
//加载过程不存在多线程，没必要使用原子变量
//#include <atomic>

class File_IO_Base
{
public:
    enum FILE_IO_FORM {
        STREAM,
        MMAP,
    };

    File_IO_Base(){};
    File_IO_Base(const std::string filename);
    const std::string get_filename();
    virtual int open(FILE_IO_FORM form , const std::string&& filename) = 0;
    virtual const char* read(FILE_IO_FORM form, size_t len) = 0;
    virtual int write(const std::string &&content) = 0;
    virtual int close(FILE_IO_FORM form ) = 0;
    virtual bool exist(const std::string& filename) = 0;
    virtual uintmax_t size(const std::string& filename) = 0;
protected:
    std::string filename;
    std::shared_ptr<std::fstream> stream;
    int fd;
    loff_t offset;
};


class File_IO : public File_IO_Base
{
public:
    File_IO(const std::string filename,const std::string back_filename) :File_IO_Base(filename) {
        this->back_filename = back_filename;
        this->Is_Loading = false;
    }

    File_IO(const File_IO*& other)
    {
        this->filename = other->filename;
        this->fd = other->fd;
        this->offset = other->offset;
        this->stream = std::move(other->stream);
        this->back_filename = other->back_filename;
        this->Is_Loading = other->Is_Loading;
    }

    File_IO(const File_IO&& other)
    {
        this->filename = other.filename;
        this->fd = other.fd;
        this->offset = other.offset;
        this->stream = std::move(other.stream);
        this->back_filename = other.back_filename;
        this->Is_Loading = other.Is_Loading;
    }

    int open(FILE_IO_FORM form  ,const std::string&& filename) override;
    const char* read(FILE_IO_FORM form , size_t len) override;
    int write(const std::string &&content) override;
    int close(FILE_IO_FORM form ) override;
    bool exist(const std::string& filename) override;
    uintmax_t size(const std::string& filename) override;
    void reset(FILE_IO_FORM form);
    void erase_from_end(int line_size);

    bool Is_Loading;
    std::string back_filename;
};