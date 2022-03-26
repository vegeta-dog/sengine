#pragma once

#include "../utils/logger/logger.h"
#include "../utils/kafka-cpp/kafka_client.h"
#include "../database/DataBase.h"
#include <mysql/mysql.h>
#include <hiredis/hiredis.h>
#include <mutex>
#include <map>

#include "InvertedIndex.h"

namespace indexBuilder
{
    class builder
    {
    public:
        builder(Database::DataBase *db);
        ~builder();

        void run();

        void worker();

    private:
        Database::DataBase *db;
        logging::logger *log;
    };

    /**
     * @brief 启动索引构建器模块
     *
     */
    void run();

    /**
     * @brief 执行启动索引构建器对象的操作
     *
     * @param db 数据库对象
     */
    void do_start(Database::DataBase *db);

    /**
     * @brief 从内容评估器接收消息的回调函数
     *
     * @param rec
     */
    void message_recv_from_Eva_handler(kafka::clients::consumer::ConsumerRecord rec);

    /**
     * @brief 预处理网页，为每个网页构建倒排列表
     * 
     * @param inv_lists 
     */
    void preprocess(std::map<std::string, indexBuilder::InvertedIndex::InvertedIndex_List *> &inv_map, boost::json::object &msg_obj);
}