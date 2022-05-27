#pragma once

#include "../utils/logger/logger.h"
#include "../utils/kafka-cpp/kafka_client.h"
#include "../database/DataBase.h"
#include "../indexBuilder/InvertedIndex.h"
#include <mysql/mysql.h>
#include <hiredis/hiredis.h>

namespace Searcher
{

    static std::mutex mtx_Searcher_id;
    static unsigned int max_Searcher_id = 0;

    class searcher
    {
    public:
        searcher(Database::DataBase *db);
        ~searcher();
        void run();

    private:
        /**
         * @brief 读取倒排索引文件
         *
         * @param key 关键字
         * @return indexBuilder::InvertedIndex::InvertedIndex_List
         */
        indexBuilder::InvertedIndex::InvertedIndex_List read_inv_index(const std::string &key);

        /**
         * @brief 采用dfs来检查待选网页集合
         *
         * @param key_num 当前正在处理的关键词的序号
         * @param key_arr_size 最大关键词序号
         * @param idWebPage 当前正在处理的网页号
         * @param base_offset 基础offset
         * @param max_delta 关键词之间的最大间隔值
         * @param inv_node_map 备选倒排索引结点map
         * @param its 备选倒排索引列表迭代器数组
         * @param its_end 备选单词的倒排索引列表的end()迭代器数组
         * @return true 该网页符合要求
         * @return false 该网页不符合要求
         */
        bool dfs_check_relationship(const unsigned int &key_num, const unsigned int &key_arr_size, const unsigned int &idWebPage, const unsigned int &base_offset, const unsigned int &max_delta, std::map<std::string, std::list<indexBuilder::InvertedIndex::list_node *> *> &inv_node_map, std::vector<std::list<indexBuilder::InvertedIndex::list_node *>::iterator> &its, std::vector<std::list<indexBuilder::InvertedIndex::list_node *>::iterator> &its_end);

        /**
         * @brief 输出检索结果到redis
         *
         * @param req_id 用户请求的id
         * @param res_pid_set 最终文章的结果集
         */
        void output_result(const std::string &req_id, std::set<unsigned int> &res_pid_set, std::map<unsigned int, unsigned int> &idWebPage_keycount_map);

        Database::DataBase *db;
        logging::logger *log;
        unsigned int searcher_id;
    };

    /**
     * @brief 启动内容检索器模块
     *
     */
    void run();

    /**
     * @brief 执行启动searcher对象的操作
     *
     * @param db 数据库对象
     */
    void do_start(Database::DataBase *db);

    /**
     * @brief 从分词模块接收消息的回调函数
     *
     * @param rec
     */
    void message_recv_from_WordSplit(kafka::clients::consumer::ConsumerRecord rec);

    unsigned int get_Searcher_id();
}