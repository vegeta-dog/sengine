//
// Created by longjin on 2022/3/8.
//
#pragma once
#ifndef SENGINE_REDIS_POOL_H
#define SENGINE_REDIS_POOL_H

#include <string>
#include <mutex>
#include <hiredis/hiredis.h>
#include "logger/logger.h"
#include "iostream"

#define redis_pool_CONN_NULL 1
#define redis_pool_CONN_FLAG_NULL 2

namespace DataBase::RedisPool {
    /**
 * redis conn连接池
 * 由于hiRedis不是线程安全的，因此需要线程池
 */
    static logging::logger log_redis_pool("redis_pool");

    class redis_pool {

    public:
        redis_pool() = default;

        ~redis_pool() {
            for (int i = 0; i < conn_num; ++i) {
                if (conn_pool[i] != NULL) {
                    redisFree(conn_pool[i]);
                    conn_pool[i] = NULL;
                }

            }
            delete[] conn_pool;

            if (conn_flag != NULL) {
                delete[] conn_flag;
                conn_flag = NULL;
            }
        }

        int init(std::string ip_, int port_, int conn_num_) {
            ip = ip_;
            port = port_;
            conn_num = conn_num_;


            conn_pool = new redisContext *[conn_num];
            if (conn_pool == NULL) {
                log_redis_pool.error(__LINE__, "Cannot allocate redisContext group!");
                return redis_pool_CONN_NULL;
            }

            conn_flag = new bool[conn_num];
            if (conn_flag == NULL) {
                log_redis_pool.error(__LINE__, "Cannot allocate conn_flag!");
                return redis_pool_CONN_FLAG_NULL;
            }

            for (int i = 0; i < conn_num; ++i) {
                conn_pool[i] = redisConnect(ip.c_str(), port);
                if (conn_pool[i] == NULL || conn_pool[i]->err) {
                    char buf[256];
                    sprintf(buf, "redisConnect can not be established! Error code:%d", conn_pool[i]->err);
                    log_redis_pool.error(__LINE__, buf);
                    return 3;
                }

                conn_flag[i] = false;
            }

            empty_num = conn_num;
            current_conn = 0;
            log_redis_pool.info(__LINE__, "Successfully initialized redis conn pool!");
            return 0;
        }

        redisContext *get_conn(int &id) {
            if (empty_num == 0) {
                return NULL;
            }

            mtx.lock();

            while (conn_flag[current_conn] != 0) { current_conn = (current_conn + 1) % conn_num; }

            conn_flag[current_conn] = true;
            --empty_num;
            id = current_conn;
            current_conn = (current_conn + 1) % conn_num;

            mtx.unlock();

            return conn_pool[id];
        }

        void put_conn(int id) {
            if (id < conn_num && id >= 0) {
                mtx.lock();

                conn_flag[id] = 0;
                ++empty_num;

                mtx.unlock();
            }

        }

    private:
        std::string ip;
        int port;
        int conn_num;

        redisContext **conn_pool;
        bool *conn_flag;
        int empty_num;
        int current_conn;

        std::mutex mtx;

    };

}

#endif //SENGINE_REDIS_POOL_H
