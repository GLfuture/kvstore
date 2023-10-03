#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
using std::string;
using std::string_view;
using std::vector;
class Split
{
public:
    Split(string buffer)
    {
        this->buffer = buffer;
    }
    void Spilt_With_Char(const char symbol)
    {
        string_view view = buffer;
        for (int i = 0; i < view.size(); i++)
        {
            int beg = i;
            i = view.find(symbol, beg);
            string_view sub_view = view.substr(beg, i - beg);
            result.push_back(sub_view);
            if (i == string::npos)
                break;
        }
    }

    void Spilt_With_Str(const string symbol)
    {
        string_view view = buffer;
        size_t len = symbol.length();
        for (int i = 0; i < view.size(); i++)
        {
            int beg = i;
            i = view.find(symbol, beg);
            string_view sub_view = view.substr(beg, i - beg);
            result.push_back(sub_view);
            if (i == string::npos)
                break;
            i += len - 1;
        }
    }

    // 用一个字符分割字符串，单引号中的内容视为一个整体，单引号错误返回false，\'进行转义
    bool Spilt_With_Char_Connect_With_Quote(const char partition)
    {
        return Spilt_With_Char_Connect_With_char(partition, '\'');
    }

    // 用一个字符分割字符串，双引号中的内容视为一个整体,双引号错误返回false，\"进行转移
    bool Spilt_With_Char_Connect_With_Double_Quote(const char partition)
    {
        return Spilt_With_Char_Connect_With_char(partition, '\"');
    }

    void Set_Buffer(string buffer)
    {
        this->buffer = buffer;
    }

    void Clean_Buffer()
    {
        buffer.clear();
    }

    vector<string_view> Get_Result()
    {
        return result;
    }

private:
    bool Spilt_With_Char_Connect_With_char(const char partition, const char connction)
    {
        string_view view = buffer;
        int beg = 0, end = 0;
        uint16_t status = 0;
        for (int i = 0; i < view.size(); i++)
        {
            if (view[i] == '\\')
            {
                i++;
                end++;
                if (i > view.size())
                {
                    result.clear();
                    return false;
                }
                continue;
            }
            else if (view[i] == connction)
            {
                if (status == 0)
                {
                    beg = i + 1;
                    end++;
                    status = 1;
                    continue;
                }
                else
                {
                    if (beg != end)
                        result.push_back(view.substr(beg, end - beg));
                    end++;
                    beg = end;
                    status = 0;
                    continue;
                }
            }
            else if (view[i] == partition && status == 0)
            {
                if (beg != end)
                    result.push_back(view.substr(beg, end - beg));
                beg = end + 1;
            }
            end++;
            if (i == view.size() - 1)
            {
                if (beg != end)
                    result.push_back(view.substr(beg, end - beg));
            }
        }
        if (status)
        {
            result.clear();
            return false;
        }
        return true;
    }

    void Clean_Result()
    {
        result.clear();
    }

private:
    /*
    result提供查看字符串的视图，这样，原字符串不会受到影响，只是相当于将buffer分隔查看
    */

    string buffer;
    vector<string_view> result;
};