#pragma once

#include <string>
namespace libUTF
{
    /**
     * @brief 截取含有utf字符的字符串
     * 
     * @param begin 起始位置
     * @param len 结束位置
     * @return std::string 结果串 
     */
    std::string substr_utf(const std::string & data, int begin, int end);
    std::string substr_utf_bylen(const std::string&, int, int);
}