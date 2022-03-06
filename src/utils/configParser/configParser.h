//
// Created by longjin on 2022/3/5.
//

#ifndef SENGINE_CONFIGPARSER_H
#define SENGINE_CONFIGPARSER_H

#include<map>
#include<boost/property_tree/ptree.hpp>
#include<boost/property_tree/ini_parser.hpp>
/**
 * 定义错误码
 */
// 执行成功
#define SUCCESS 0
// 文件路径错误
#define E_PATH_ERROR 1
// 配置文件格式错误
#define E_SYNTAX_ERROR



namespace configParser {


/** 解析配置文件
 * @param path 文件路径
 * @param conf_str string类型的参数
 * @param conf_int int类型的参数
 * @param conf_double 浮点类型的参数
 * @return
 */
    int parse(const std::string &path);

/**
 * 获取配置项
 * @param key 关键字，格式为 <section, key_name>
 * @param data 返回数据的指针
 */
    int get_config(const std::string &key, void* data);

}


#endif //SENGINE_CONFIGPARSER_H
