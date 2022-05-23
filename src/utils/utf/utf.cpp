#include "utf.h"
#include<string.h>
/**
 * @brief 截取含有utf字符的字符串
 *
 * @param begin 起始位置
 * @param len 结束位置
 * @return std::string 结果串
 */
std::string libUTF::substr_utf(const std::string & data, int begin, int len)
{
 
    const char* chs = data.c_str();
    int strl = strlen(chs);
    while (begin < strl && begin < len)
    {
        begin += ((unsigned int)chs[begin] > 0x80) ? 3 : 1;
    }
    if (begin > data.length())
    {
        begin = data.length();
    }
    return data.substr(0, begin);
}