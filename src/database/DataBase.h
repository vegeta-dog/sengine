//
// Created by longjin on 2022/03/05.
//

#ifndef SENGINE_DATABASE_H
#define SENGINE_DATABASE_H


#include<mysql/mysql.h>

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


        MYSQL *conn = mysql_init(NULL);


    };

    void example();

}


#endif //SENGINE_DATABASE_H
