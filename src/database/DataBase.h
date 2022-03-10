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

        ::DataBase::RedisPool::redis_pool *redis_conn_pool;
        ::DataBase::MysqlPool::mysql_pool *mysql_conn_pool;
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
}


#endif //SENGINE_DATABASE_H
