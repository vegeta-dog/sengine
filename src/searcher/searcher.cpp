#include "searcher.h"

#include "../utils/configParser/configParser.h"
#include "../utils/queue/queue.h"
#include "../utils/kafka-cpp/kafka_client.h"

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/json.hpp>

namespace bj = boost::json;

static std::list<boost::thread *> threads;
static std::list<Searcher::searcher *> searhcer_objs;

static ThreadSafeQueue::queue<std::string> recv_from_WS_queue;

void Searcher::run()
{
    configParser::parse("config.ini");
    Database::DataBase db;

    std::string str;
    configParser::get_config("Searcher.Searcher_num", &str);
    unsigned int indexBuilder_num = boost::lexical_cast<int>(str);

    boost::thread *t;
    Searcher::searcher *searcher_ptr;

    for (unsigned int i = 0; i < indexBuilder_num; ++i)
    {
        t = new boost::thread(&Searcher::do_start, &db);
        threads.emplace_back(t);
    }

    // 启动kafka客户端
    configParser::get_config("Kafka.kafka_brokers", &str);
    t = new boost::thread(&Kafka_cli::do_start_kafka_consumer, str, "WordSplit2Searcher", "Searcher_recv_WS", &Searcher::message_recv_from_WordSplit);
    threads.emplace_back(t);

    for (auto x = threads.begin(); x != threads.end(); ++x)
        (*x)->join();
}

void Searcher::do_start(Database::DataBase *db)
{
    Searcher::searcher sc(db);
    searhcer_objs.emplace_back(&sc);
    sc.run();
}

void Searcher::message_recv_from_WordSplit(kafka::clients::consumer::ConsumerRecord rec)
{
    recv_from_WS_queue.push(rec.value().se_toString());
}

Searcher::searcher::searcher(Database::DataBase *db)
{
    this->db = db;
    this->searcher_id = Searcher::get_Searcher_id();
    this->log = new logging::logger("Searcher " + boost::lexical_cast<std::string>(this->searcher_id));
}

Searcher::searcher::~searcher()
{
    free(this->log);
}

void Searcher::searcher::run()
{
    while (true)
    {
        try
        {
            // 解析json
            std::string msg;
            try
            {
                msg = recv_from_WS_queue.getFront();
            }
            catch (int e)
            {
                if (e == -1)
                    usleep(100);
                else
                    std::cerr << "At line " << __LINE__ << ": unexpected exception" << std::endl;
                continue;
            }
            std::cerr << "msg=" + msg << std::endl;

            bj::value jv = bj::parse(msg);
            boost::json::object msg_obj = jv.as_object();
            boost::json::array key_arr = msg_obj.at("content").as_array();

            std::map<std::string, indexBuilder::InvertedIndex::InvertedIndex_List> inv_map;

            int set_flag = 1; // set flag=1时插入ws1，否则插入ws0
            bool flag_init = true;
            std::set<unsigned int> working_set[2];

            unsigned int key_arr_size = 0;
            // 读取关键词的倒排列表,并求交集
            for (const auto &x : key_arr)
            {
                ++key_arr_size;
                auto key = boost::lexical_cast<std::string>(x.as_string());
                log->warn(__LINE__, "key=" + key);
                std::cerr << "key=" + key << std::endl;
                inv_map[key] = this->read_inv_index(key);

                if (flag_init)
                {
                    flag_init = false;
                    working_set[0] = inv_map[key].page_set;

                    continue;
                }
                for (const auto &y : inv_map[key].page_set)
                {
                    if (working_set[(set_flag ? 0 : 1)].count(y))
                    {
                        working_set[set_flag].insert(y);
                    }
                }

                set_flag = (set_flag ? 0 : 1);
                working_set[set_flag].clear();
            }
            set_flag = (set_flag ? 0 : 1); // 最终具有所有关键词的页面的集合

            // ========= 提取页面的倒排结点 =========
            std::map<std::string, std::list<indexBuilder::InvertedIndex::list_node *> *> inv_node_map; // 记得释放动态申请的list的内存

            std::map<unsigned int, unsigned int> idWebPage_keycount_map; // 网页id——关键词总出现次数 映射
            // 提前将map中涉及到的项置零
            for (const auto &x : working_set[set_flag])
                idWebPage_keycount_map[x] = 0;

            for (const auto &x : key_arr)
            {
                auto key = bj::value_to<std::string>(x);
                indexBuilder::InvertedIndex::InvertedIndex_List *ptr = &inv_map[key];

                inv_node_map[key] = new std::list<indexBuilder::InvertedIndex::list_node *>;

                for (auto &y : ptr->list)
                {
                    indexBuilder::InvertedIndex::list_node *p = &y;
                    if (working_set[set_flag].count(y.idWebPage))
                    {
                        inv_node_map[key]->emplace_back(p);
                        ++idWebPage_keycount_map[y.idWebPage]; // 计算每个网页中，总匹配的关键词次数
                    }
                }
            }

            std::vector<std::list<indexBuilder::InvertedIndex::list_node *>::iterator> its;     // inv_node_map中，每个key的倒排结点列表的当前搜索页面的base iterator（指向该页面的第一个node）
            std::vector<std::list<indexBuilder::InvertedIndex::list_node *>::iterator> its_end; // inv_node_map中，每个key的倒排结点列表的end迭代器
            for (unsigned int i = 0; i < key_arr_size; ++i)
            {
                its.emplace_back(inv_node_map[bj::value_to<std::string>(key_arr[i])]->begin());
                its_end.emplace_back(inv_node_map[bj::value_to<std::string>(key_arr[i])]->end());
            }

            std::set<unsigned int> result_page_id_set;
            // ========= 暴力搜索，查询是否存在按顺序的关键词关系 =====
            for (const auto &pid : working_set[set_flag])
            {
                if (this->dfs_check_relationship(0, key_arr_size, pid, (*its[0])->offset, 3, inv_node_map, its, its_end))
                {
                    // 当前网页能够被加入结果集
                    result_page_id_set.insert(pid);
                }

                // 将所有的迭代器指向下一个网页
                for (unsigned int i = 0; i < key_arr_size; ++i)
                {
                    while ((*its[i])->idWebPage == pid)
                        ++(its[i]);
                }
            }

            // 释放inv_node_map的动态申请的内存
            for (auto &x : inv_node_map)
            {
                free(x.second);
            }

            // 输出结果
            this->output_result(boost::lexical_cast<std::string>(msg_obj.at("id").as_string()), result_page_id_set);
        }
        catch (const std::exception &e)
        {
            this->log->error(__LINE__, boost::lexical_cast<std::string>(e.what()));
        }
    }
}

/**
 * @brief 读取倒排索引文件
 *
 * @param key 关键字
 * @return indexBuilder::InvertedIndex::InvertedIndex_List
 */
indexBuilder::InvertedIndex::InvertedIndex_List Searcher::searcher::read_inv_index(const std::string &key)
{
    int mysql_conn_id;
    MYSQL *mysql_conn = this->db->mysql_conn_pool->get_conn(mysql_conn_id);

    std::string sql = "SELECT path FROM InvertedIndexTable WHERE InvertedIndexTable.key='" + key + "';";
    if (mysql_query(mysql_conn, sql.c_str()))
    {
        log->error(__LINE__,
                       "mysql query failed. Message:" +
                           boost::lexical_cast<std::string>(mysql_error(mysql_conn)));
        this->db->mysql_conn_pool->free_conn(mysql_conn_id);
        throw "mysql query failed.";
    }

    MYSQL_RES *res;

    res = mysql_store_result(mysql_conn);

    MYSQL_ROW column;

    if (mysql_affected_rows(mysql_conn) != 1)
    {
        log->error(__LINE__, "mysql affectedlines != 1.");
        this->db->mysql_conn_pool->free_conn(mysql_conn_id);
        throw "mysql affectedlines != 1.";
    }

    std::string path;
    while (column = mysql_fetch_row(res))
    {
        path = boost::lexical_cast<std::string>(column[0]);
    }

    // 读取倒排列表
    indexBuilder::InvertedIndex::InvertedIndex_List inv_list;

    std::ifstream fin(path, std::ios::in);
    boost::archive::binary_iarchive ia(fin);
    ia >> inv_list;
    fin.close();

    this->db->mysql_conn_pool->free_conn(mysql_conn_id);
    return inv_list;
}

/**
 * @brief 输出检索结果到redis
 *
 * @param req_id 用户请求的id
 * @param res_pid_set 最终文章的结果集
 */
void Searcher::searcher::output_result(const std::string &req_id, std::set<unsigned int> &res_pid_set)
{
    int redis_conn_id;
    redisContext *redis_conn = this->db->redis_conn_pool->get_conn(redis_conn_id);

    int mysql_conn_id;
    MYSQL *mysql_conn = this->db->mysql_conn_pool->get_conn(mysql_conn_id);

    // 从mysql中查询数据
    // todo: 输出检索结果
    // 内容摘要截取文章前140个字
    std::string base_sql = "SELECT LEFT(document,140),  title, url FROM WebPage WHERE idWebPage=";

    MYSQL_RES *res;

    res = mysql_store_result(mysql_conn);

    MYSQL_ROW column;

    boost::json::array arr_ret2api;
    bj::object ret2api;

    for (const auto &id : res_pid_set)
    {
        if (mysql_query(mysql_conn, (base_sql + boost::lexical_cast<std::string>(id) + ";").c_str()))
        {
            log->error(__LINE__, "mysql execute error! SQL: " + (base_sql + boost::lexical_cast<std::string>(id) + ";"));
            this->db->mysql_conn_pool->free_conn(mysql_conn_id);
            this->db->redis_conn_pool->free_conn(redis_conn_id);
            throw "mysql execute error! SQL: " + (base_sql + boost::lexical_cast<std::string>(id) + ";");
        }

        res = mysql_store_result(mysql_conn);

        if (mysql_affected_rows(mysql_conn) != 1)
        {
            log->error(__LINE__, "mysql affectedlines != 1.");
            this->db->mysql_conn_pool->free_conn(mysql_conn_id);
            this->db->redis_conn_pool->free_conn(redis_conn_id);
            throw "mysql affectedlines != 1.";
        }

        while (column = mysql_fetch_row(res))
        {
            bj::object tmp;
            tmp["title"] = column[1];
            tmp["url"] = column[2];
            tmp["summary"] = column[0];
            arr_ret2api.emplace_back(tmp);
        }
    }

    time_t t;
    time(&t);
    ret2api["objects"] = arr_ret2api;
    ret2api["timestamp"] = t;

    // 输出json到redis

    redisReply *reply = (redisReply *)redisCommand(redis_conn, ("SET search:" + (req_id) + " " + bj::serialize(ret2api)).c_str());
    if (reply == NULL)
    {
        log->error(__LINE__, "redis error: msg: " + boost::lexical_cast<std::string>(reply->str));
    }
    freeReplyObject(reply);

    this->db->mysql_conn_pool->free_conn(mysql_conn_id);
    this->db->redis_conn_pool->free_conn(redis_conn_id);
}

unsigned int Searcher::get_Searcher_id()
{
    unsigned int ret;
    mtx_Searcher_id.lock();

    ret = max_Searcher_id++;

    mtx_Searcher_id.unlock();
    return ret;
}

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
bool Searcher::searcher::dfs_check_relationship(const unsigned int &key_num, const unsigned int &key_arr_size, const unsigned int &idWebPage, const unsigned int &base_offset, const unsigned int &max_delta, std::map<std::string, std::list<indexBuilder::InvertedIndex::list_node *> *> &inv_node_map, std::vector<std::list<indexBuilder::InvertedIndex::list_node *>::iterator> &its, std::vector<std::list<indexBuilder::InvertedIndex::list_node *>::iterator> &its_end)
{
    if (key_num >= key_arr_size)
    {
        auto it = its[key_num - 1];
        // 将当前的指针移动到下一个page
        while ((*it)->idWebPage == idWebPage && (it != its_end[key_num - 1]))
            ++it;

        its[key_num - 1] = it;
        return true;
    }

    auto it = its[key_num];

    while ((it != its_end[key_num]) && ((*it)->idWebPage == idWebPage))
    {
        if (abs((long long)((*it)->offset - base_offset)) <= max_delta)
        {

            if (dfs_check_relationship(key_num + 1, key_arr_size, idWebPage, (*it)->offset, max_delta, inv_node_map, its, its_end))
                return true;
            else
            {
                // 将当前的指针移动到下一个page
                while ((*it)->idWebPage == idWebPage && (it != its_end[key_num]))
                    ++it;

                its[key_num] = it;
                return false;
            }
        }
    }

    // 将当前的指针移动到下一个page
    while ((*it)->idWebPage == idWebPage && (it != its_end[key_num]))
        ++it;

    its[key_num] = it;
    return false;
}