//
// Created by longjin on 2022/3/5.
//

#ifndef SENGINE_CONFIGPARSER_H
#define SENGINE_CONFIGPARSER_H

#include<map>

/**
 * 定义错误码
 */
// 执行成功
#define SUCCESS 0
// 文件路径错误
#define PATH_ERROR 1
// 配置文件格式错误
#define SYNTAX_ERROR


namespace configParser {


    /** 解析配置文件
     * @param path 文件路径
     * @param conf_str string类型的参数
     * @param conf_int int类型的参数
     * @param conf_double 浮点类型的参数
     * @return
     */
    int parse(const std::string& path, std::map<std::string, std::string> *conf_str, std::map<std::string, int> *conf_int,
              std::map<std::string, double> *conf_double);
}


#endif //SENGINE_CONFIGPARSER_H
