//
// Created by longjin on 2022/03/05.
//

#ifndef SENGINE_DATABASE_H
#define SENGINE_DATABASE_H


#include<mysql/mysql.h>
#include "redis_pool.h"

#include "logger/logger.h"

namespace Database {

    class DataBase {
    public:
        DataBase();

        ~DataBase();

    private:
        std::string mysql_host;
        int mysql_port;
        std::string mysql_username;
        std::string mysql_password;
        std::string mysql_database_name;

        std::string redis_host;
        int redis_port;
        std::string redis_username;
        std::string redis_password;
        int redis_max_conn_num;


        MYSQL *conn = mysql_init(NULL);

        redis_pool* redis_conn_pool;

    };

    void example();

}


#endif //SENGINE_DATABASE_H
