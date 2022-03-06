//
// Created by longjin on 2022/03/05.
//

#include "DataBase.h"
#include "iostream"
#include<configParser/configParser.h>

#include <boost/lexical_cast.hpp>

#include<stdlib.h>


logging::logger log_DataBase("DataBase");

void Database::example()
{
    std::cout<<"xxxxx"<<std::endl;
}

Database::DataBase::~DataBase() {
    //this->session.close();
    //free(this->session);
}

Database::DataBase::DataBase() {
    log_DataBase.info(__LINE__, "Initializing DataBase Object...");
    std::string str_data;
    int int_data;
    configParser::get_config("DataBase.mysql_host", &str_data);
    this->mysql_host = str_data;

    std::cout<<"123"<<std::endl;
    configParser::get_config("DataBase.mysql_port", &str_data);
    char* ptr;
    this->mysql_port = atoi(str_data.c_str());
    std::cout<<this->mysql_port<<std::endl ;
    configParser::get_config("DataBase.mysql_username", &str_data);
    this->mysql_username = str_data;

    configParser::get_config("DataBase.mysql_password", &str_data);
    this->mysql_password = str_data;

    configParser::get_config("DataBase.mysql_database_name", &str_data);
    this->mysql_database_name = str_data;



    driver = sql::mysql::get_mysql_driver_instance();
    if(driver == NULL)
    {
        log_DataBase.error(__LINE__, "driver is NULL.");
        exit(0);
    }

    conn = driver->connect("tcp://"+ this->mysql_host+":"+ boost::lexical_cast<std::string>(this->mysql_port)+"/" + this->mysql_database_name,
                           this->mysql_username, this->mysql_password);

    if(conn == NULL)
    {
        log_DataBase.error(__LINE__, "conn is NULL.");
        exit(0);
    }

    log_DataBase.info(__LINE__, "Successfully connected to MySQL server!");



}
