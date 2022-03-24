//
// Created by longjin on 2022/3/23.
//

#ifndef SENGINE_EVALUATOR_H
#define SENGINE_EVALUATOR_H
#include "../utils/logger/logger.h"
#include "../utils/kafka/kafka_client.h"
#include "../database/DataBase.h"
#include <mysql/mysql.h>
#include <hiredis/hiredis.h>
#include <mutex>

namespace Evaluator
{
    static std::mutex mtx_evaluator_id;
    static unsigned int max_evaluator_id = 0;
    static unsigned int get_evaluator_id();

    class evaluator
    {
    public:
        /**
         * @brief Construct a new evaluator object
         *
         * @param mysql_conn mysql连接
         * @param mysql_conn_id mysql连接号
         * @param redis_conn redis连接
         * @param redis_conn_id redis连接号
         */
        evaluator(MYSQL *mysql_conn, unsigned int mysql_conn_id, redisContext *redis_conn, unsigned int redis_conn_idsssss, Database::DataBase*db);
        ~evaluator();

        /**
         * @brief 评估器的执行函数
         * 
         */
        void run();

    private:
        MYSQL *mysql_conn;
        redisContext *redis_conn;
        unsigned int mysql_conn_id;
        unsigned int redis_conn_id;

        unsigned int id;
        Database::DataBase *db;
        logging::logger * log;
    };

    /**
     * @brief kafka client收到数据的回调函数
     *
     * @param rec
     */
    void message_handler(kafka::clients::consumer::ConsumerRecord rec);

    /**
     * 启动evaluator模块
     */
    void run();
}

#endif // SENGINE_EVALUATOR_H
