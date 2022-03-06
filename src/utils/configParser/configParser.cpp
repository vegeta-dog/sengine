//
// Created by longjin on 2022/3/5.
//

#include "configParser.h"
#include<cstdio>
#include<cstdlib>
#include<stdexcept>
#include<iostream>
#include<boost/property_tree/ptree.hpp>
#include<boost/property_tree/ini_parser.hpp>


logging::logger log_configParser("configParser");

boost::property_tree::ptree config;
#define GET_SECTION(str)                    \
(                                           \
    str.substr(0, str.find('.'))   \
)

/** 解析配置文件
 * @param path 文件路径
 * @param conf_str string类型的参数
 * @param conf_int int类型的参数
 * @param conf_double 浮点类型的参数
 * @return
 */
int configParser::parse(const std::string &path) {

    try {
        boost::property_tree::ini_parser::read_ini(path, config);
    }
    catch (const std::runtime_error &e) {
        log_configParser.error(__LINE__, e.what());
    }
    return SUCCESS;
};
/**
 * 获取配置项
 * @param key 关键字，格式为 <section, key_name>
 * @param data 返回数据的指针
 */

int configParser::get_config(const std::string &key, void *data) {

    std::string type = GET_SECTION(key);

    if (type == "int")
        *(int *) data = config.get<int>(key);
    else if (type == "double")
        *(double *) data = config.get<double>(key);
    else if (type == "float")
        *(float *) data = config.get<float>(key);
    else
        *(std::string *) data = config.get<std::string>(key);
    return SUCCESS;
}
