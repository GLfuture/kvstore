#pragma once
#ifndef CONFIG_H
#define CONFIG_H
#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string.h>
#include <yaml-cpp/yaml.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <type_traits>
#include <vector>
#include <spdlog/spdlog.h>

namespace Config_NSP {
    class Config_Base
    {
    public:
      virtual int Load_Conf(const std::string& filepath) = 0; 
      virtual ~Config_Base(){}
    protected:
      int Read_Conf(const std::string& filepath,std::string &result);
    };

    //解析json配置文件
    class Config_Json : public Config_Base
    {
    public:
      int Load_Conf(const std::string& filepath) override;

      template<class T>
      inline int Get_Value(const rapidjson::Value& root,const std::string& key,T& value)
      {
        spdlog::error("Get_Value function : argument value type error");
        return -1;
      }

      rapidjson::Value& Get_Root_Value()
      {
        return m_root;
      }
    private:
      rapidjson::Value m_root;
    };

    class Config_Yaml : public Config_Base
    {
    public:
      int Load_Conf(const std::string& filepath) override;

      template <class T>
      inline int Get_Value(const YAML::Node &root,const std::string& key ,T& value)
      {
        if(root[key.c_str()].IsNull()){
          spdlog::info("Get_Value funcion : yaml has no member");
          return -1;
        }
        value = root[key.c_str()].as<T>();
        return 0;
      }

      YAML::Node &Get_Root_Value(){
        return m_root;
      }
    private:
      YAML::Node m_root;
    };

}

//Config_Json的特化
//int 特化
template <>
inline int Config_NSP::Config_Json::Get_Value<int>(const rapidjson::Value& root,const std::string& key,int& value)
{
  if(root.HasMember(key.c_str())){
    if(root[key.c_str()].IsInt()){
      value = root[key.c_str()].GetInt();
    }else{
      spdlog::info("Get_Value function : value's type does not match to json's type");
    }
  }else{
    spdlog::info("Get_Value function : json has no member");
    return -1;
  }
  return 0;
}

//uint 特化
template <>
inline int Config_NSP::Config_Json::Get_Value<uint>(const rapidjson::Value& root,const std::string& key,uint& value)
{
  if(root.HasMember(key.c_str())){
    if(root[key.c_str()].IsUint()){
      value = root[key.c_str()].GetUint();
    }else{
      spdlog::info("Get_Value function : value's type does not match to json's type");
    }
  }else{
    spdlog::info("Get_Value function : json has no member");
    return -1;
  }
  return 0;
}

//int64特化
template <>
inline int Config_NSP::Config_Json::Get_Value<int64_t>(const rapidjson::Value& root,const std::string& key,int64_t& value)
{
  if(root.HasMember(key.c_str())){
    if(root[key.c_str()].IsInt64()){
      value = root[key.c_str()].GetInt64();
    }else{
      spdlog::info("Get_Value function : value's type does not match to json's type");
    }
  }else{
    spdlog::info("Get_Value function : json has no member");
    return -1;
  }
  return 0;
}

//uint64特化
template <>
inline int Config_NSP::Config_Json::Get_Value<uint64_t>(const rapidjson::Value& root,const std::string& key,uint64_t& value)
{
  if(root.HasMember(key.c_str())){
    if(root[key.c_str()].IsUint64()){
      value = root[key.c_str()].GetUint64();
    }else{
      spdlog::info("Get_Value function : value's type does not match to json's type");
    }
  }else{
    spdlog::info("Get_Value function : json has no member");
    return -1;
  }
  return 0;
}

//float特化
template <>
inline int Config_NSP::Config_Json::Get_Value<float>(const rapidjson::Value& root,const std::string& key,float& value)
{
  if(root.HasMember(key.c_str())){
    if(root[key.c_str()].IsFloat()){
      value = root[key.c_str()].GetFloat();
    }else{
      spdlog::info("Get_Value function : value's type does not match to json's type");
    }
  }else{
    spdlog::info("Get_Value function : json has no member");
    return -1;
  }
  return 0;
}

//double特化
template <>
inline int Config_NSP::Config_Json::Get_Value<double>(const rapidjson::Value& root,const std::string& key,double& value)
{
  if(root.HasMember(key.c_str())){
    if(root[key.c_str()].IsDouble()){
      value = root[key.c_str()].GetDouble();
    }else{
      spdlog::info("Get_Value function : value's type does not match to json's type");
    }
  }else{
    spdlog::info("Get_Value function : json has no member");
    return -1;
  }
  return 0;
}

//string特化
template <>
inline int Config_NSP::Config_Json::Get_Value<std::string>(const rapidjson::Value& root,const std::string& key,std::string& value)
{
  if(root.HasMember(key.c_str())){
    if(root[key.c_str()].IsString()){
      value = root[key.c_str()].GetString();
    }else{
      spdlog::info("Get_Value function : value's type does not match to json's type");
    }
  }else{
    spdlog::info("Get_Value function : json has no member");
    return -1;
  }
  return 0;
}

//bool特化
template <>
inline int Config_NSP::Config_Json::Get_Value<bool>(const rapidjson::Value& root,const std::string& key,bool& value)
{
  if(root.HasMember(key.c_str())){
    if(root[key.c_str()].IsBool()){
      value = root[key.c_str()].GetBool();
    }else{
      spdlog::info("Get_Value function : value's type does not match to json's type");
    }
  }else{
    spdlog::info("Get_Value function : json has no member");
    return -1;
  }
  return 0;
}

//Value特化
template <>
inline int Config_NSP::Config_Json::Get_Value<rapidjson::Value>(const rapidjson::Value& root,const std::string& key,rapidjson::Value& value)
{
  if(root.HasMember(key.c_str())){
    const rapidjson::Value& jsonval = root[key.c_str()];
    rapidjson::Document doc;
    value.CopyFrom(jsonval,doc.GetAllocator());
  }else{
    spdlog::info("Get_Value function : json has no member");
    return -1;
  }
  return 0;
}
#endif