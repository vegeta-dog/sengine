#pragma once

#include "../utils/logger/logger.h"
#include "../utils/kafka-cpp/kafka_client.h"
#include "../database/DataBase.h"

#include <mysql/mysql.h>
#include <hiredis/hiredis.h>
#include <mutex>
#include <map>
#include <string>

#include <boost/json.hpp>

#include "InvertedIndex.h"

namespace indexBuilder
{
    // 倒排索引文件的base路径
    const std::string invIndex_file_base_path = "./data/invIndex/"; // 注意，这里结尾需要有 / 符号
    class builder
    {
    public:
        builder(Database::DataBase *db);
        ~builder();

        void run();

    private:
        Database::DataBase *db;
        logging::logger *log;
    };

    /**
     * @brief 构建索引的工作线程
     *
     * @param id 网页id
     * @param key 索引的关键字
     * @param path 已经存在的索引文件的路径（若为-1，则创建新的索引）
     * @param pre_proc_list 当前关键字预处理的倒排列表
     */
    void worker(unsigned int id, std::string key, std::string path, indexBuilder::InvertedIndex::InvertedIndex_List &pre_proc_list, Database::DataBase *db, logging::logger *log);

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
    void preprocess(std::map<std::string, indexBuilder::InvertedIndex::InvertedIndex_List> &inv_map, boost::json::object &msg_obj);

    /**
     * @brief 生成倒排索引文件的k路径
     *
     * @param id 倒排索引的id
     * @return std::string 生成的路径
     */
    std::string gen_invIndex_filepath(const int &id);
}