#include "base64.h"

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <string>
#include <iostream>
#include <sstream>

/**
 * @brief 将字符串进行base64编码
 *
 * @param str 源字符串
 * @return std::string b64编码后的字符串
 */
std::string libBase64::encode(const std::string &str)
{
    typedef boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<std::string::const_iterator, 6, 8>> Base64EncodeIterator;
    std::stringstream result;
    std::copy(Base64EncodeIterator(str.begin()), Base64EncodeIterator(str.end()), std::ostream_iterator<char>(result));
    size_t equal_count = (3 - str.length() % 3) % 3;
    for (size_t i = 0; i < equal_count; i++)
    {
        result.put('=');
    }
    return result.str();
}

/*
bool Base64Decode(const string &input, string *output)
{
    typedef boost::archive::iterators::transform_width<boost::archive::iterators::binary_from_base64<string::const_iterator>, 8, 6> Base64DecodeIterator;
    stringstream result;
    try
    {
        copy(Base64DecodeIterator(input.begin()), Base64DecodeIterator(input.end()), ostream_iterator<char>(result));
    }
    catch (...)
    {
        return false;
    }
    *output = result.str();
    return output->empty() == false;
}
*/