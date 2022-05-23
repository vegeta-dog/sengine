#pragma once

#include<string>

namespace libBase64
{
    /**
     * @brief 将字符串进行base64编码
     * 
     * @param str 源字符串
     * @return std::string b64编码后的字符串 
     */
    std::string encode(const std::string & str);
} // namespace libBase64
