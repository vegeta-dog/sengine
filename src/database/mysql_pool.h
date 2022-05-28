//
// Created by longjin on 2022/3/8.
//
#pragma once
#ifndef SENGINE_MYSQL_POOL_H
#define SENGINE_MYSQL_POOL_H

#include <string>
#include <mutex>
#include<mysql/mysql.h>
#include "logger/logger.h"
#include "iostream"
#include <unistd.h>

#define E_mysql_pool_CONN_NULL 1
#define E_mysql_pool_CONN_FLAG_NULL 2
#define E_mysql_pool_CONN_CANNOT_ESTABLISH 3

namespace Database::MysqlPool {
    /**
 * mysql conn连接池
 */
    static logging::logger log_mysql_pool("MysqlPool");

    class mysql_pool {
    public:
        mysql_pool() = default;

        ~mysql_pool() {
            for (int i = 0; i < max_conn_num; ++i) {
                if (conn_pool[i] != NULL) {
                    mysql_close(conn_pool[i]);
                    conn_pool[i] = NULL;
                }

            }
            delete[] conn_pool;

            if (conn_flag != NULL) {
                delete[] conn_flag;
                conn_flag = NULL;
            }
        }

        int
        init(const std::string &h, int port_, int conn_num_, const std::string &username, const std::string &password,
             const std::string &db_name) {
            this->mysql_host = h;
            this->mysql_port = port_;
            this->mysql_username = username;
            this->mysql_password = password;
            this->mysql_database_name = db_name;

            max_conn_num = conn_num_;


            conn_pool = new MYSQL *[max_conn_num];
            if (conn_pool == NULL) {
                log_mysql_pool.error(__LINE__, "Cannot allocate MYSQL group!");
                return E_mysql_pool_CONN_NULL;
            }

            conn_flag = new bool[max_conn_num];
            if (conn_flag == NULL) {
                log_mysql_pool.error(__LINE__, "Cannot allocate conn_flag!");
                return E_mysql_pool_CONN_FLAG_NULL;
            }

            for (int i = 0; i < max_conn_num; ++i) {
                conn_pool[i] = mysql_init(NULL);
                conn_pool[i] = mysql_real_connect(conn_pool[i], this->mysql_host.c_str(), this->mysql_username.c_str(),
                                                  this->mysql_password.c_str(), this->mysql_database_name.c_str(),
                                                  this->mysql_port, NULL, 0);

                if (conn_pool[i] == NULL) {
                    log_mysql_pool.error(__LINE__, "MySQL conn is NULL.");
                    return E_mysql_pool_CONN_CANNOT_ESTABLISH;
                }

                
                mysql_set_server_option(conn_pool[i], MYSQL_OPTION_MULTI_STATEMENTS_ON);

                // 设置使用utf8
                mysql_set_character_set(conn_pool[i],"utf8");

                conn_flag[i] = false;
            }

            this->conn_num_free = max_conn_num;
            current_conn = 0;
            log_mysql_pool.info(__LINE__, "Successfully initialized mysql conn pool!");
            return 0;
        }

        /**
         * 获取连接
         * @param id 返回的数据连接的序号（凭此序号归还数据库连接）
         * @return 数据库连接
         */
        MYSQL *get_conn(int &id) {
            while(conn_num_free == 0)
            {
                usleep(100'0000);
            }

            mtx.lock();

            while (conn_flag[current_conn]) { current_conn = (current_conn + 1) % this->max_conn_num; }

            conn_flag[current_conn] = true;
            --this->conn_num_free;
            id = current_conn;
            current_conn = (current_conn + 1) % this->max_conn_num;

            mtx.unlock();

            return conn_pool[id];
        }

        void free_conn(int id) {
            if (id < this->max_conn_num && id >= 0) {
                mtx.lock();

                conn_flag[id] = false;
                ++(this->conn_num_free);

                mtx.unlock();
            }

        }

    private:
        std::string mysql_host;
        int mysql_port;
        std::string mysql_username;
        std::string mysql_password;
        std::string mysql_database_name;

        int max_conn_num;
        int conn_num_free;
        int current_conn;

        MYSQL **conn_pool;
        bool *conn_flag;

        std::mutex mtx;

    };

}

#endif //SENGINE_MYSQL_POOL_H
