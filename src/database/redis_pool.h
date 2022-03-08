//
// Created by longjin on 2022/3/8.
//

#ifndef SENGINE_REDIS_POOL_H
#define SENGINE_REDIS_POOL_H

#include <string>
#include <mutex>
#include <hiredis/hiredis.h>

/**
 * redis conn连接池
 * 由于hiRedis不是线程安全的，因此需要线程池
 */
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
            return 1;
        }

        conn_flag = new int[conn_num];
        if (conn_flag == NULL) {
            return 2;
        }

        for (int i = 0; i < conn_num; ++i) {
            conn_pool[i] = redisConnect(ip.c_str(), port);
            if (conn_pool[i] == NULL || conn_pool[i]->err) {
                return 3;
            }

            conn_flag[i] = 0;
        }

        empty_num = conn_num;
        current_conn = 0;

        return 0;
    }

    redisContext *get_conn(int &id) {
        if (empty_num == 0) {
            return NULL;
        }

        mtx.lock();

        while (conn_flag[current_conn] != 0) { current_conn = (current_conn + 1) % conn_num; }

        conn_flag[current_conn] = 1;
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

        return;
    }

private:
    std::string ip;
    int port;
    int conn_num;

    redisContext **conn_pool;
    int *conn_flag;
    int empty_num;
    int current_conn;

    std::mutex mtx;

};

#endif //SENGINE_REDIS_POOL_H
