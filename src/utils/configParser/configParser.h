//
// Created by longjin on 2022/3/5.
//
#pragma once
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

// config key not exists
#define E_KEY_NOT_EXISTS 2


#include<logger/logger.h>

namespace configParser {


/** 解析配置文件
 * @param path 文件路径
 * @return
 */
    int parse(const std::string &path);

/**
 * 从map中获取配置项
 * @param key 关键字，格式为 section.key
 * @param data 返回数据的指针
 */
    int get_config(const std::string &key, std::string *data);

    /**
     * 检测config完整性
     * @return
     */
    int check_config_integrity();

    /**
     * 从文件获取config信息
     * @param key 关键字，格式为 section.key
     * @param data 返回数据的指针
     * @return
     */
    static int get_config_from_file(const std::string &key, std::string *data);
}


#endif //SENGINE_CONFIGPARSER_H
