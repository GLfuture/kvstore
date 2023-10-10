/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-10-09 11:32:31
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-10 09:03:43
 */
#include "fileio/fileio.h"

File_IO_Base::File_IO_Base(const std::string filename)
{
    this->stream = std::make_shared<std::fstream>();
    this->filename = filename;
    this->fd = 0;
    this->offset = 0;
}

const std::string File_IO_Base::get_filename()
{
    return this->filename;
}

//以可读，追加写方式打开文件
int File_IO::open(FILE_IO_FORM form, const std::string&& filename)
{
    if(form == STREAM){
        stream->open(filename,std::fstream::in | std::fstream::out | std::fstream::app);
        if(stream->fail()){
            return errno;
        }
    }else if(form == MMAP)
    {
        fd = ::open(filename.c_str(),O_RDWR | O_CREAT);
        if( fd == -1){
            return fd;
        }
        offset = 0;
    }
    return 0;
}

//当使用mmap时，len最好为4kb的整数倍
const char* File_IO::read(FILE_IO_FORM form , size_t len)
{
    char* buffer = new char[len];
    memset(buffer,0,len);
    if(form == STREAM)
    {
        stream->read(buffer,len);
    }
    else if(form == MMAP)
    {
        buffer = (char *)mmap64(NULL,len,PROT_READ,MAP_SHARED,fd,offset);
        offset += len;
    }
    return buffer;
}

int File_IO::write(const std::string &&content)
{

    stream->write(content.c_str(), content.length());
    stream->flush();
    return stream->sync();
}

int File_IO::close(FILE_IO_FORM form)
{
    if(form == STREAM)
    {
        stream->close();
    }
    else if(form == MMAP)
    {
        ::close(fd);
    }
    return 0;
}

bool File_IO::exist(const std::string &filename)
{
    return std::filesystem::exists(filename);
}

uintmax_t File_IO::size(const std::string &filename)
{
    return std::filesystem::file_size(filename);
}

void File_IO::reset(FILE_IO_FORM form)
{
    if(form == STREAM)
    {
        stream->seekg(std::fstream::beg);
    }
    else if(form == MMAP)
    {
        offset = 0;
    }
}