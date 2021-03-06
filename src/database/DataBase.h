//
// Created by longjin on 2022/03/05.
//
#pragma once
#ifndef SENGINE_DATABASE_H
#define SENGINE_DATABASE_H


#include<mysql/mysql.h>
#include "redis_pool.h"
#include "mysql_pool.h"
#include "logger/logger.h"


#define ON 1
#define OFF 0

namespace Database {

    class DataBase {
    public:
        DataBase();

        ~DataBase();

        Database::RedisPool::redis_pool *redis_conn_pool;
        Database::MysqlPool::mysql_pool *mysql_conn_pool;
    private:
        std::string mysql_host;
        int mysql_port;
        std::string mysql_username;
        std::string mysql_password;
        int mysql_max_conn_num;

        std::string mysql_database_name;
        std::string redis_host;
        int redis_port;
        std::string redis_password;
        int redis_max_conn_num;


    };

    /**
     * 初始化mysql数据库的数据表
     * @return
     */
    int init_mysql(DataBase &db);

    /**
     * @brief 将redis命令中的引号进行转义
     * 
     * @param x 源字符串
     * @return std::string  转义后的结果
     */
    std::string translate_quote_for_redis(const std::string &x);
}


#endif //SENGINE_DATABASE_H
