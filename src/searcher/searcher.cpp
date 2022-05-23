#include "searcher.h"

#include "../utils/configParser/configParser.h"
#include "../utils/kafka-cpp/kafka_client.h"
#include "../utils/queue/queue.h"

#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

namespace bj = boost::json;

static std::list<boost::thread *> threads;
static std::list<Searcher::searcher *> searhcer_objs;

static ThreadSafeQueue::queue<std::string> recv_from_WS_queue;

/**
 * @brief
 *
 * @param l
 * @param r
 * @return int
 */
static int randInt(int l, int r)
{
    static std::mt19937 eng(time(0) ^ 19937 ^ 998344353);
    std::uniform_int_distribution<int> dis(l, r);
    return dis(eng);
}

/**
 * @brief 删除字符串前后的引号，使得数据合法
 *
 * @param item  原始字符串
 * @return std::string
 */
static std::string remove_pre_suf_quote(const std::string &item)
{
    if (item.length() <= 2)
        throw "can't normalize a empty string or item.length <= 2";

    if (item[0] == '\"' && item[item.length() - 1] == '\"')
        return item.substr(1, (int)item.length() - 2);
    else
        return item;
}

// use of 'auto' in parameter declaration only available with -fconcepts
static std::string key_to_string(auto val)
{
    return remove_pre_suf_quote(boost::lexical_cast<std::string>(val.as_string()));
}

void Searcher::run()
{
    configParser::parse("config.ini");
    Database::DataBase db;

    std::string str;
    configParser::get_config("Searcher.Searcher_num", &str);
    unsigned int searcher_num = boost::lexical_cast<int>(str);

    boost::thread *t;
    Searcher::searcher *searcher_ptr;

    for (unsigned int i = 0; i < searcher_num; ++i)
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
                    usleep(100'000);
                else
                    std::cerr << "At line " << __LINE__ << ": unexpected exception" << std::endl;
                continue;
            }

            bj::value jv = bj::parse(msg);
            boost::json::object msg_obj = jv.as_object();
            boost::json::array key_arr = msg_obj.at("content").as_array();

            std::cout << jv << std::endl;

            std::map<std::string, indexBuilder::InvertedIndex::InvertedIndex_List> inv_map;

            int set_flag = 1;                      // set flag=1时插入ws1，否则插入ws0
            bool flag_init = true;                 // 是不是循环的第一次
            std::set<unsigned int> working_set[2]; // 求交集

            unsigned int key_arr_size = 0;
            // 读取关键词的倒排列表,并求交集
            for (const auto &x : key_arr)
            {

                auto key = remove_pre_suf_quote(boost::lexical_cast<std::string>(x.as_string()));

                // 如果已经查过,就不需要重复查了
                if (inv_map.count(key))
                    continue;

                ++key_arr_size;

                log->warn(__LINE__, "key=" + key);
                auto inv_res = this->read_inv_index(key);

                if (inv_res.list.empty())
                    continue;
                inv_map[key] = inv_res;

                std::cerr << "get inv_index success!" << std::endl;

                if (flag_init)
                {
                    flag_init = false;
                    working_set[0] = inv_map[key].page_set;
                    continue;
                }
                for (const auto &y : inv_map[key].page_set)
                {
                    if (working_set[(set_flag ? 0 : 1)].count(y) || randInt(0, 100) % 5 == 0)
                    {
                        working_set[set_flag].insert(y);
                    }
                }

                set_flag = (set_flag ? 0 : 1);
                working_set[set_flag].clear();
            }
            set_flag ^= 1; // 最终具有所有关键词的页面的集合

            // ========= 提取页面的倒排结点 =========
            std::map<std::string, std::list<indexBuilder::InvertedIndex::list_node *> *> inv_node_map; // 记得释放动态申请的list的内存
            // inv_node_map: 把(所有'属于交集里面的页'的倒排节点)取出来, 目的是:降低后面的计算数据规模

            std::map<unsigned int, unsigned int> idWebPage_keycount_map; // 网页id——关键词总出现次数 映射
            // 提前将map中涉及到的项置零
            for (const auto &x : working_set[set_flag])
                idWebPage_keycount_map[x] = 0;

            for (const auto &x : key_arr)
            {
                auto key = remove_pre_suf_quote(boost::lexical_cast<std::string>(x.as_string()));

                // 倒排链表
                indexBuilder::InvertedIndex::InvertedIndex_List *ptr = &inv_map[key];

                inv_node_map[key] = new std::list<indexBuilder::InvertedIndex::list_node *>;

                for (auto &y : ptr->list) // y是一个倒排节点
                {
                    indexBuilder::InvertedIndex::list_node *p = &y;
                    if (working_set[set_flag].count(y.idWebPage))
                    {
                        inv_node_map[key]->emplace_back(p);
                        // 计算每个网页中，总匹配的关键词次数
                        ++idWebPage_keycount_map[y.idWebPage];
                    }
                }
            }

            // inv_node_map中，每个key的倒排结点列表的当前搜索页面的base iterator（指向该页面的第一个node）
            std::vector<std::list<indexBuilder::InvertedIndex::list_node *>::iterator> its;
            // inv_node_map中，每个key的倒排结点列表的end迭代器
            std::vector<std::list<indexBuilder::InvertedIndex::list_node *>::iterator> its_end;
            for (unsigned int i = 0; i < key_arr_size; ++i)
            {
                its.emplace_back(inv_node_map[key_to_string(key_arr[i])]->begin());
                its_end.emplace_back(inv_node_map[key_to_string(key_arr[i])]->end());
            }

            std::set<unsigned int> result_page_id_set;
            // ========= 暴力搜索，查询是否存在按顺序的关键词关系 =====

            const int match_delta = 33; // 匹配间隔为50 ???

            // 从页面的交集里面枚举页面id
            for (const auto &pid : working_set[set_flag])
            {
                // 传入pid, 查看这个页面是否
                if (this->dfs_check_relationship(0, key_arr_size, pid, 0, match_delta, inv_node_map, its, its_end))
                {
                    // 当前网页能够被加入结果集
                    result_page_id_set.insert(pid);
                }

                // 将所有的迭代器指向下一个网页
                for (unsigned int i = 0; i < key_arr_size; ++i)
                {
                    while (its[i] != its_end[i] && (*its[i])->idWebPage == pid)
                        ++(its[i]);
                }
            }

            // 释放inv_node_map的动态申请的内存
            for (auto &x : inv_node_map)
                free(x.second);

            // 输出结果
            std::string query_str_id = key_to_string(msg_obj.at("id"));
            this->output_result(query_str_id, result_page_id_set);
        }
        catch (const std::exception &e)
        {
            this->log->error(__LINE__, boost::lexical_cast<std::string>(e.what()));
        }
    }
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
bool Searcher::searcher::dfs_check_relationship(
    const unsigned int &key_num,
    const unsigned int &key_arr_size,
    const unsigned int &idWebPage,
    const unsigned int &base_offset,
    const unsigned int &max_delta,
    std::map<std::string, std::list<indexBuilder::InvertedIndex::list_node *> *> &inv_node_map, std::vector<std::list<indexBuilder::InvertedIndex::list_node *>::iterator> &its,
    std::vector<std::list<indexBuilder::InvertedIndex::list_node *>::iterator> &its_end)
{
    if (key_num >= key_arr_size)
    {
        auto it = its[key_num - 1];
        // 将当前的指针移动到下一个page (避免重复枚举到一个page)
        while ((it != its_end[key_num - 1]) && (*it)->idWebPage == idWebPage)
            ++it;

        its[key_num - 1] = it; // 这里是存储最后一个its吗 ？
        return true;
    }

    auto it = its[key_num];

    while ((it != its_end[key_num]) && ((*it)->idWebPage == idWebPage))
    {
        if (key_num == 0 || abs((long long)((*it)->offset - base_offset)) <= max_delta)
        {

            if (dfs_check_relationship(key_num + 1, key_arr_size, idWebPage, (*it)->offset, max_delta, inv_node_map, its, its_end))
                return true;
            else
            {
                // 将当前的指针移动到下一个page
                while ((it != its_end[key_num]) && (*it)->idWebPage == idWebPage)
                    ++it;

                its[key_num] = it;
                return false;
            }
        }
        else if ((long long)(*it)->offset < base_offset)
        {
            ++it;
        }
        else
        {
            break;
        }
    }

    // 将当前的指针移动到下一个page
    while ((it != its_end[key_num]) && (*it)->idWebPage == idWebPage)
        ++it;

    its[key_num] = it;
    return false;
}

/**
 * @brief 读取倒排索引文件
 *
 * @param key 关键字
 * @return indexBuilder::InvertedIndex::InvertedIndex_List
 */
indexBuilder::InvertedIndex::InvertedIndex_List // return 一个 list
Searcher::searcher::read_inv_index(const std::string &key)
{
    // 返回的list
    indexBuilder::InvertedIndex::InvertedIndex_List inv_list;

    std::cerr << "inside head of read_inv_index" << std::endl;

    int mysql_conn_id;
    MYSQL *mysql_conn = this->db->mysql_conn_pool->get_conn(mysql_conn_id);

    std::string sql = "SELECT path FROM InvertedIndexTable WHERE InvertedIndexTable.key='" + key + "';";

    std::cerr << "start sql query!" << std::endl;

    if (mysql_query(mysql_conn, sql.c_str()))
    {
        log->error(__LINE__, "mysql query failed. Message:" +
                                 boost::lexical_cast<std::string>(mysql_error(mysql_conn)));
        this->db->mysql_conn_pool->free_conn(mysql_conn_id);
        throw "mysql query failed.";
    }

    std::cerr << "sql query ok" << std::endl;

    MYSQL_RES *res;

    res = mysql_store_result(mysql_conn);

    MYSQL_ROW column;

    std::cerr << "mysql store result ok!" << std::endl;

    int rows = mysql_affected_rows(mysql_conn);
    if (rows > 1)
    {
        log->error(__LINE__, "mysql affectedlines != 1. affects:" + boost::lexical_cast<std::string>(mysql_affected_rows(mysql_conn)));
        this->db->mysql_conn_pool->free_conn(mysql_conn_id);
        throw "mysql affectedlines != 1.";
    }
    else if (rows == 0) // 没有索引
    {
        return inv_list;
    }

    std::cerr << "start mysql fetch row !" << std::endl;

    std::string path;
    while (column = mysql_fetch_row(res))
    {
        path = boost::lexical_cast<std::string>(column[0]);
    }

    if (path.empty())
    {
        throw "path can't be empty!";
    }

    // 读取倒排列表

    std::ifstream fin(path, std::ios::in);
    boost::archive::binary_iarchive ia(fin);
    ia >> inv_list;
    fin.close();

    std::cerr << "OK ! return from read_inv_index" << std::endl;

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
            log->error(__LINE__, "mysql affectedlines != 1. num=" + boost::lexical_cast<std::string>(mysql_affected_rows(mysql_conn)));
            this->db->mysql_conn_pool->free_conn(mysql_conn_id);
            this->db->redis_conn_pool->free_conn(redis_conn_id);
            throw "mysql affectedlines != 1.";
        }

        while (column = mysql_fetch_row(res))
        {
            bj::object tmp;

            std::cerr << sizeof(column) << std::endl;
            // std::cerr << "column[0] = " << column[0] << std::endl;
            // std::cerr << "column[1] = " << column[1] << std::endl;
            // URL为空
            if (column[2] == NULL)
                continue;

            tmp["url"] = column[2];
            if (column[1] == NULL)
                tmp["title"] = "title";
            else
                tmp["title"] = column[1];

            if (column[0] == NULL)
                tmp["summary"] = "summary";
            else
                tmp["summary"] = column[0];

            arr_ret2api.emplace_back(tmp);
        }
    }

    time_t t;
    time(&t);
    ret2api["objects"] = arr_ret2api;
    ret2api["timestamp"] = t;

    // 输出json到redis
    std::stringstream redis_cmd;
    redis_cmd << "SET search:" << (req_id) << " \"" << Database::translate_quote_for_redis(bj::serialize(ret2api)) << "\"";

    redisReply *reply = (redisReply *)redisCommand(redis_conn, redis_cmd.str().c_str());
    if (reply == NULL)
    {
        std::cerr << "redis_command=  " << redis_cmd.str().c_str() << std::endl;
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