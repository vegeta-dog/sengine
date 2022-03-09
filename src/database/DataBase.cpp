//
// Created by longjin on 2022/03/05.
//

#include "DataBase.h"
#include "iostream"
#include<configParser/configParser.h>

#include <boost/lexical_cast.hpp>

#include<stdlib.h>


logging::logger log_DataBase("DataBase");


Database::DataBase::~DataBase() {

    log_DataBase.info(__LINE__, "Database object destructed.");
}

Database::DataBase::DataBase() {
    log_DataBase.info(__LINE__, "Initializing DataBase Object...");

    std::string str_data;
    configParser::get_config("DataBase.mysql_host", &str_data);
    this->mysql_host = str_data;


    configParser::get_config("DataBase.mysql_port", &str_data);
    this->mysql_port = boost::lexical_cast<int>(str_data);

    configParser::get_config("DataBase.mysql_pool_max_conn", &str_data);
    this->mysql_max_conn_num = boost::lexical_cast<int>(str_data);

    configParser::get_config("DataBase.mysql_username", &str_data);
    this->mysql_username = str_data;

    configParser::get_config("DataBase.mysql_password", &str_data);
    this->mysql_password = str_data;

    configParser::get_config("DataBase.mysql_database_name", &str_data);
    this->mysql_database_name = str_data;

    configParser::get_config("DataBase.redis_host", &str_data);
    this->redis_host = str_data;

    configParser::get_config("DataBase.redis_port", &str_data);
    this->redis_port = boost::lexical_cast<int>(str_data);

    configParser::get_config("DataBase.redis_username", &str_data);
    this->redis_username = str_data;

    configParser::get_config("DataBase.redis_password", &str_data);
    this->redis_password = str_data;

    configParser::get_config("DataBase.redis_pool_max_conn", &str_data);
    this->redis_max_conn_num = boost::lexical_cast<int>(str_data);


    // 初始化MySQL连接池
    this->mysql_conn_pool = new ::DataBase::MysqlPool::mysql_pool();
    this->mysql_conn_pool->init(this->mysql_host, this->mysql_port, this->mysql_max_conn_num, this->mysql_username,
                                this->mysql_password, this->mysql_database_name);



    /**
     * 初始化redis连接池
     */
    this->redis_conn_pool = new ::DataBase::RedisPool::redis_pool();
    int errcode = 0;
    errcode = this->redis_conn_pool->init(this->redis_host, boost::lexical_cast<int>(this->redis_port),
                                          boost::lexical_cast<int>(this->redis_max_conn_num));
    if (errcode) {
        char code[128];
        sprintf(code, "Failed to establish redis Connect. code: %d", errcode);
        log_DataBase.error(__LINE__, code);
    }

}
