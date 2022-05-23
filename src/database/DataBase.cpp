//
// Created by longjin on 2022/03/05.
//

#include "DataBase.h"
#include "iostream"
#include <fstream>
#include <configParser/configParser.h>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <stdlib.h>
#include <vector>

logging::logger log_DataBase("DataBase");

Database::DataBase::~DataBase()
{

    log_DataBase.debug(__LINE__, "Database object destructed.");

    // 显式调用析构函数
    this->mysql_conn_pool->~mysql_pool();
    this->redis_conn_pool->~redis_pool();
}

Database::DataBase::DataBase()
{
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


    configParser::get_config("DataBase.redis_password", &str_data);
    this->redis_password = str_data;

    configParser::get_config("DataBase.redis_pool_max_conn", &str_data);
    this->redis_max_conn_num = boost::lexical_cast<int>(str_data);

    int errcode = 0;

    // 初始化MySQL连接池
    this->mysql_conn_pool = new Database::MysqlPool::mysql_pool();
    errcode = this->mysql_conn_pool->init(this->mysql_host, this->mysql_port, this->mysql_max_conn_num,
                                          this->mysql_username,
                                          this->mysql_password, this->mysql_database_name);
    if (errcode) {
        char code[128];
        sprintf(code, "Failed to create mysql conn pool. code: %d", errcode);
        log_DataBase.error(__LINE__, code);
        exit(0);
    }
    /**
     * 初始化redis连接池
     */
    this->redis_conn_pool = new Database::RedisPool::redis_pool();

    errcode = this->redis_conn_pool->init(this->redis_host, boost::lexical_cast<int>(this->redis_port),
                                          boost::lexical_cast<int>(this->redis_max_conn_num), this->redis_password);
    if (errcode) {
        char code[128];
        sprintf(code, "Failed to create redis conn pool. code: %d", errcode);
        log_DataBase.error(__LINE__, code);
        exit(0);
    }
}

/**
 * 初始化mysql数据库的数据表
 * @return 错误码
 */
int Database::init_mysql(DataBase &db) {
    int conn_id;
    MYSQL *conn = db.mysql_conn_pool->get_conn(conn_id);

    log_DataBase.info(__LINE__, "Initializing MySQL data tables...");

    // 先读取sql语句
    std::string cwd = boost::filesystem::initial_path<boost::filesystem::path>().string();
    std::ifstream fin(cwd + "/database/sql/create_database.sql");

    std::vector<std::string> sql;
    std::string lineStr;
    while (std::getline(fin, lineStr))
        sql.emplace_back(lineStr);

    // 关闭自动提交，开启事务
    mysql_autocommit(conn, OFF);
    bool flag_error = false;

    int js = -1;
    std::string sql_to_exec = "";
    for (std::string s: sql) {

        ++js;
        if (!s.empty()) {

            // 拼接sql语句
            if (s[s.length() - 1] != ';' && s[0] != '-' && s[1] != '-') {
                sql_to_exec += s + ' ';
                continue;
            } else if (s[s.length() - 1] == ';')
                sql_to_exec += s;
            else continue;

            if (mysql_query(conn, sql_to_exec.c_str())) {
                // 处理出错
                flag_error = true;
                break;
            }

            sql_to_exec = "";

        }
    }

    // 处理出错
    if (flag_error) {
        char buf[256];
        sprintf(buf, "An error occurred while creating MySQL data table. At Line %d", js);
        log_DataBase.error(__LINE__, buf);
        // 数据回滚
        mysql_rollback(conn);
        exit(0);
    } else mysql_commit(conn);

    // 重新开启自动提交
    mysql_autocommit(conn, ON);

    log_DataBase.info(__LINE__, "Successfully created sEngine Data tables!");
    return SUCCESS;
}