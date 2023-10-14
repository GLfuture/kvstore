/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-10-12 20:48:44
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-14 16:20:04
 */
#include "config/config.h"

int Config_NSP::Config_Base::Read_Conf(const std::string &filepath, std::string &result)
{
    if (!std::filesystem::exists(filepath))
    {
        return errno;
    }
    std::ifstream fin;
    fin.open(filepath);
    if (fin.fail())
    {
        return errno;
    }
    fin.seekg(0, std::ios_base::end);
    std::streampos len = fin.tellg();
    fin.seekg(0, std::ios_base::beg);
    char *buffer = new char[len];
    memset(buffer, 0, len);
    fin.read(buffer, len);
    result.assign(buffer);
    return 0;
}

int Config_NSP::Config_Json::Load_Conf(const std::string &filepath)
{
    std::string content = "";
    int ret = Read_Conf(filepath, content);
    if (!!ret)
    {
        spdlog::error("Read Json Config Error : {}", std::string(strerror(ret)));
        return -1;
    }
    rapidjson::Document doc;
    doc.Parse(content.c_str());
    if (doc.HasParseError())
    {
        spdlog::error("Json Parse Error : ErrorCode is {}", std::to_string(doc.GetParseError()));
        return -1;
    }
    m_root.CopyFrom(doc,doc.GetAllocator());
    return 0;
}

int Config_NSP::Config_Yaml::Load_Conf(const std::string &filepath)
{
    
    m_root = YAML::LoadFile(filepath);
    return 0;
}
