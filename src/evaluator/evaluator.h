//
// Created by longjin on 2022/3/23.
//

#ifndef SENGINE_EVALUATOR_H
#define SENGINE_EVALUATOR_H

#include "../utils/logger/logger.h"
#include "../utils/kafka-cpp/kafka_client.h"
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
        evaluator(MYSQL *mysql_conn, int mysql_conn_id, redisContext *redis_conn, int redis_conn_id, Database::DataBase *db);
        ~evaluator();

        /**
         * @brief 评估器的执行函数
         *
         */
        void run();

    private:
        /**
         * @brief 检查url是否在数据库内
         *
         * @param url
         * @return true
         * @return false
         */
        bool check_url_in_db(const std::string &url);

        /**
         * @brief 将网页链接存入数据库
         *  若网页链接不在数据库中，则存入数据库，否则直接返回链接的id
         * @param url
         * @return 主键ID
         */
        int store_weblink2db(const std::string &url);

        /**
         * @brief 创建网页指向关系记录
         *
         * @param from  来源网页的id
         * @param to 被指向的网页的id
         * @return true 成功创建
         * @return false 创建失败
         */
        bool create_LinkRecord(unsigned int from, unsigned int to);

        MYSQL *mysql_conn;
        redisContext *redis_conn;
        unsigned int mysql_conn_id;
        unsigned int redis_conn_id;

        unsigned int id;
        Database::DataBase *db;
        logging::logger *log;
    };

    /**
     * @brief kafka client收到数据的回调函数
     *
     * @param rec
     */
    void message_handler(kafka::clients::consumer::ConsumerRecord rec);

    /**
     * @brief 向索引构建器发送数据的handler，从queue中读取数据并返回给producer
     *
     * @return std::string
     */
    std::string send_msg2indexBuilder_handler();

    /**
     * 启动evaluator模块
     */
    void run();

    /**
     * @brief 真正启动评估器
     *
     */
    void do_start(MYSQL *mysql_conn, int mysql_conn_id, redisContext *redis_conn, int redis_conn_id, Database::DataBase *db);

}

#endif // SENGINE_EVALUATOR_H
