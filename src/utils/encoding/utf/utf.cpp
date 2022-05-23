#include "utf.h"
#include <string.h>

/**
 * @brief 截取含有utf字符的字符串 左闭右开
 *
 * @param begin 起始位置 (起始位置是0)
 * @param end 结束位置  (结尾位置是length)
 * @return std::string 结果串
 */
std::string libUTF::substr_utf(const std::string &data, int begin, int end)
{
    // 传入utf8字符的第一个字节,可以知道这个字符有多少个byte
    auto char_size = [](const unsigned int& bit) -> unsigned int {  
        return bit >= 0x80 ? 3 : 1;
    };
    const char *chs = data.c_str();
    int strl = strlen(chs), l = 0, r = 0;
    for (int i = 0; l < strl && i < begin; i++)
        l += char_size(chs[l]);
    for (int i = 0; r < strl && i < end; i++) 
        r += (r == strl) ? 1 : char_size(chs[r]);
    return data.substr(l, r - l);
}


/**
 * @brief 截取含有utf字符的字符串
 *
 * @param begin 起始位置 (起始位置是0)
 * @param len 长度  
 * @return std::string 结果串
 */
std::string libUTF::substr_utf_bylen(const std::string &data, int begin, int len)
{
    return substr_utf(data, begin, begin + len);
}

