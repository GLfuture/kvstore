/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-30 14:09:41
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-01 03:58:41
 */
#include "fileio/fileio.h"

FileIO_Base::FileIO_Base(const std::string &path)
{
    this->_path = std::move(path);
}

File_IO::File_IO(const std::string &path): FileIO_Base(path)
{

}

int File_IO::Read(std::string &content)
{
    if (!std::filesystem::exists(_path))
    {
        return errno;
    }
    _read_in.open(_path);
    if (_read_in.fail())
    {
        return errno;
    }
    _read_in.seekg(0, std::ios_base::end);
    std::streampos len = _read_in.tellg();
    _read_in.seekg(0, std::ios_base::beg);
    char *buffer = new char[len];
    memset(buffer, 0, len);
    _read_in.read(buffer, len);
    content.assign(buffer);
    _read_in.close();
    return 0;
}
int File_IO::Write(const std::string &buffer)
{
}