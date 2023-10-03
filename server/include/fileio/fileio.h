/*
 * @Description: 文件IO
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-30 14:02:28
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-01 06:05:35
 */
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <string.h>

class FileIO_Base
{
public:
    FileIO_Base(const std::string& path);
    virtual int Read(std::string& content) = 0;
    virtual int Write(const std::string& buffer) = 0;
    virtual ~FileIO_Base() = default;
protected:
    std::string _path;
};

class File_IO:public FileIO_Base
{
public:
    File_IO(const std::string& path);
    int Read(std::string& content) override;
    int Write(const std::string& buffer) override;

    const std::ifstream& Get_Read_Stream_Obj() { return this->_read_in;  }

private:
    std::string _path;
    std::ifstream _read_in;
    std::ofstream _write_out;
};