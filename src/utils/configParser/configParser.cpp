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
#include<boost/lexical_cast.hpp>

#define BOOST_NO_CXX11_SCOPED_ENUMS

#include<boost/filesystem.hpp>

#define MAX_CONFIG_NUM 10

logging::logger log_configParser("configParser");

boost::property_tree::ptree config;
#define GET_SECTION(str)                    \
(                                           \
    str.substr(0, str.find('.'))   \
)

static std::pair<std::string, bool> config_req[MAX_CONFIG_NUM] =
        {
                std::make_pair("DataBase.mysql_host", true),
                std::make_pair("DataBase.mysql_port", false),
                std::make_pair("DataBase.mysql_username", true),
                std::make_pair("DataBase.mysql_password", false),
                std::make_pair("DataBase.mysql_database_name", true),
                std::make_pair("DataBase.redis_host", true),
                std::make_pair("DataBase.redis_port", false),
                std::make_pair("DataBase.redis_username", false),
                std::make_pair("DataBase.redis_password", false),
                std::make_pair("DataBase.redis_pool_max_conn", false),

        };

/**
 *  各种参数的默认值
 */
static std::map<std::string , std::string > config_default_val =
        {
                {"DataBase.mysql_port", "3306"},
                {"DataBase.redis_port", "6379"},
                {"DataBase.redis_username", ""},
                {"DataBase.redis_password",""},
                {"DataBase.redis_pool_max_conn", "10"}

        };

/** 解析配置文件
 * @param path 文件路径
 * @param conf_str string类型的参数
 * @param conf_int int类型的参数
 * @param conf_double 浮点类型的参数
 * @return
 */
int configParser::parse(const std::string &path) {
    if (!boost::filesystem::exists(path)) {
        char buf[256];
        sprintf(buf, "Config file not exists: %s", path.c_str());
        log_configParser.error(__LINE__, buf);
        exit(0);
    }
    try {
        boost::property_tree::ini_parser::read_ini(path, config);
    }
    catch (const std::runtime_error &e) {
        log_configParser.error(__LINE__, e.what());
    }

    // 进行config检查
    configParser::check_config_integrity();
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

int configParser::check_config_integrity() {
    log_configParser.info(__LINE__, "Starting config check...");

    std::pair<std::string, bool> *req = &config_req[0];
    char buf[256];

    std::string data;

    for (int i = 0; i < MAX_CONFIG_NUM; ++i) {
        sprintf(buf, "Checking config: %s...", req->first.c_str());
        log_configParser.info(__LINE__, buf);

        data = "";
        get_config(req->first, &data);
        //log_configParser.info(__LINE__, boost::lexical_cast<std::string>(data.empty()));

        // you should not use Clion's auto code simplify func here!!! Otherwise, the code here can go wrong!
        if (data.empty()) {
            if (req->second) {
                sprintf(buf, "Invalid config key:%s, it cannot be NULL!", req->first.c_str());
                log_configParser.error(__LINE__, buf);
                exit(0);
            }
            else
            {
                sprintf(buf, "Config key:%s is empty, use default value： %s", req->first.c_str(), config_default_val[req->first].c_str());
                log_configParser.warn(__LINE__, buf);
                // 设置默认值
                config.get_child(req->first).put_value(config_default_val[req->first]);
            }

        }

        ++req;
    }
    log_configParser.info(__LINE__, "Config integrity check completed!");
    return SUCCESS;
}

