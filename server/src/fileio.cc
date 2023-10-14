/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-10-09 11:32:31
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-13 20:23:33
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
        stream->open(filename,std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
        if(stream->fail()){
            std::cout<<"stream open failed\n"<<std::endl;
            return errno;
        }
    }else if(form == MMAP)
    {
        fd = ::open(filename.c_str(),O_RDWR | O_CREAT);
        if( fd == -1){ 
            std::cout<<"mmap open failed\n"<<std::endl;
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

void File_IO::erase_from_end(int line_size)
{
    int fileDescriptor = ::open(filename.c_str(), O_RDWR);
    if (fileDescriptor == -1) {
        spdlog::error("erase_from_size:open failed\n");
        return;
    }

    // 获取文件大小
    struct stat fileInfo;
    if (fstat(fileDescriptor, &fileInfo) == -1) {
        spdlog::error("erase_from_size: fstat failed\n");
        ::close(fileDescriptor);
        return;
    }
    off_t fileSize = fileInfo.st_size;

    // 将文件映射到内存
    char* fileData = static_cast<char*>(mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0));
    if (fileData == MAP_FAILED) {
        spdlog::error("erase_from_size: mmap failed\n");
        ::close(fileDescriptor);
        return;
    }

    int newFileSize = 0;
    // 从文件末尾开始删除指定行数
    int linesToDelete = -1;
    for (off_t i = fileSize - 1; i >= 0; --i) {
        if (fileData[i] == '\n') {
            ++linesToDelete;
            if (linesToDelete == line_size) {
                // 删除到达指定行数
                memset(&fileData[i + 1], 0, fileSize - i - 1);
                newFileSize = i + 1;
                break;
            }
        }
    }

    if (munmap(fileData, fileSize) == -1) {
       spdlog::error("erase_from_size: munmap failed\n");
    }

    if (ftruncate(fileDescriptor, newFileSize) == -1) {
        std::cerr << "文件截断失败。" << std::endl;
        ::close(fileDescriptor);
        return;
    }
    // 关闭文件
    ::close(fileDescriptor);

}