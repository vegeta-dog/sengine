#include "indexBuilder.h"

#include "../utils/configParser/configParser.h"
#include "../utils/queue/queue.h"
#include "../utils/kafka-cpp/kafka_client.h"
#include "../libs/bundle/bundle.h"

#include <fstream>
#include <algorithm>
#include <time.h>

#include <boost/lockfree/queue.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/filesystem.hpp>
#include <boost/serialization/vector.hpp>

namespace bj = boost::json;
namespace bio = boost::iostreams;

static logging::logger log_evaluator("IndexBuilder");

static std::list<boost::thread *> threads;
static std::list<indexBuilder::builder *> builder_objs;

static ThreadSafeQueue::queue<std::string> recv_from_eva_queue;

static std::string get_inv_index_path(const std::string &key, logging::logger *log, MYSQL *mysql_conn);

indexBuilder::builder::builder(Database::DataBase *db)
{
    this->db = db;
    this->builder_id = indexBuilder::get_indexBuilder_id();
    this->log = new logging::logger("indexBuilder " + boost::lexical_cast<std::string>(this->builder_id));
}

indexBuilder::builder::~builder()
{
    free(this->log);
}

void indexBuilder::run()
{
    configParser::parse("config.ini");
    Database::DataBase db;

    std::string str;
    configParser::get_config("indexBuilder.indexBuilder_num", &str);
    unsigned int indexBuilder_num = boost::lexical_cast<int>(str);

    boost::thread *t;
    indexBuilder::builder *builder_ptr;

    for (unsigned int i = 0; i < indexBuilder_num; ++i)
    {
        t = new boost::thread(&indexBuilder::do_start, &db);
        threads.emplace_back(t);
    }

    // 启动kafka客户端
    configParser::get_config("Kafka.kafka_brokers", &str);
    t = new boost::thread(&Kafka_cli::do_start_kafka_consumer, str, "Evaluator2indexBuilder", "indexBuilder_recv_Eva", &indexBuilder::message_recv_from_Eva_handler);
    threads.emplace_back(t);

    for (auto x = threads.begin(); x != threads.end(); ++x)
        (*x)->join();
}

void indexBuilder::message_recv_from_Eva_handler(kafka::clients::consumer::ConsumerRecord rec)
{
    recv_from_eva_queue.push(rec.value().toString());
}

/**
 * @brief 执行启动索引构建器对象的操作
 *
 * @param db 数据库对象
 */
void indexBuilder::do_start(Database::DataBase *db)
{
    indexBuilder::builder bd(db);
    builder_objs.emplace_back(&bd);
    bd.run();
}

void indexBuilder::builder::run()
{
    while (true)
    {
        try
        {
            // 解析json
            std::string msg;
            try
            {
                msg = recv_from_eva_queue.getFront();
            }
            catch (int e)
            {
                if (e == -1)
                    usleep(100);
                else
                    std::cerr << "At line " << __LINE__ << ": unexpected exception" << std::endl;
                continue;
            }

            bj::value jv = bj::parse(msg);
            boost::json::object msg_obj = jv.as_object();

            std::map<std::string, indexBuilder::InvertedIndex::InvertedIndex_List> inv_map;
            // 为当前网页创建倒排列表
            preprocess(inv_map, msg_obj);

            unsigned int page_id = msg_obj.at("id").as_uint64();
            int tmp_conn_id;
            MYSQL *tmp_conn;
            boost::thread *t;

            std::vector<boost::thread *> work_threads;

            // 分批次读取磁盘上的倒排列表到内存中，并进行合并，然后输出最终结果
            for (auto x : inv_map)
            {
                tmp_conn = this->db->mysql_conn_pool->get_conn(tmp_conn_id);

                t = new boost::thread(&worker, msg_obj, page_id, x.first, get_inv_index_path(x.first, this->log, tmp_conn), x.second, this->db, this->log);
                this->db->mysql_conn_pool->free_conn(tmp_conn_id);
                work_threads.emplace_back(t);

                // 同时执行5个线程
                if (work_threads.size() == 5)
                {
                    for (auto tt : work_threads)
                        tt->join();

                    for (auto tt : work_threads)
                        free(tt);
                    work_threads.clear();
                }
            }
            // 不够5个线程时，直接执行
            for (auto tt : work_threads)
                tt->join();

            for (auto tt : work_threads)
                free(tt);
            work_threads.clear();
        }
        catch (const std::exception &e)
        {
            this->log->error(__LINE__, boost::lexical_cast<std::string>(e.what()));
        }
    }
}

/**
 * @brief 预处理网页，为每个网页构建倒排列表
 *
 * @param inv_map 返回的倒排列表对象的映射
 * @param msg_obj 消息json对象指针
 */
void indexBuilder::preprocess(std::map<std::string, indexBuilder::InvertedIndex::InvertedIndex_List> &inv_map, boost::json::object &msg_obj)
{
    boost::json::array arr[2];
    arr[0] = msg_obj.at("title").as_array();
    arr[1] = msg_obj.at("content").as_array();

    std::list<std::string> keys;

    unsigned int id = msg_obj.at("id").as_uint64();

    unsigned int offset = 0;
    for (const auto &ar : arr)
        for (boost::json::value item : ar)
        {
            std::string str = bj::value_to<std::string>(item);

            if (!inv_map.count(str)) // 当前网页还没有统计过这个key，创建倒排列表
            {
                keys.emplace_back(str);
                inv_map[str] = indexBuilder::InvertedIndex::InvertedIndex_List(str);
                inv_map[str].page_set.insert(id);
            }

            inv_map[str].list.emplace_back(indexBuilder::InvertedIndex::list_node(id, offset));

            offset += str.length();
        }

    // 对倒排列表进行排序

    for (const std::string &x : keys)
    {

        std::sort(inv_map[x].list.begin(), inv_map[x].list.end(), indexBuilder::InvertedIndex::cmp_list_node);
    }
}

/**
 * @brief 构建索引的工作线程
 *
 * @param id 网页id
 * @param key 索引的关键字
 * @param path 已经存在的索引文件的路径（若为-1，则创建新的索引）
 * @param pre_proc_list 当前关键字预处理的倒排列表
 */
void indexBuilder::worker(bj::object &msg_obj, unsigned int id, std::string key, std::string path, indexBuilder::InvertedIndex::InvertedIndex_List &pre_proc_list, Database::DataBase *db, logging::logger *log)
{
    int mysql_id;
    MYSQL *mysql_conn = NULL;

    if (path != "-1")
    {
        indexBuilder::InvertedIndex::InvertedIndex_List existsed_list;

        std::ifstream fin(path, std::ios::in);
        boost::archive::binary_iarchive ia(fin);
        ia >> existsed_list;
        fin.close();

        // 先清空属于该网页的倒排索引
        for (auto it = existsed_list.list.begin(); it != existsed_list.list.end();)
        {
            if (it->idWebPage == id)
                it = existsed_list.list.erase(it);
            else
                ++it;
        }
        if (existsed_list.page_set.count(id))
            existsed_list.page_set.erase(id);

        // 合并索引
        auto it_pre = pre_proc_list.list.begin();
        for (auto it = existsed_list.list.begin(); it != existsed_list.list.end(); ++it)
        {
            if (indexBuilder::InvertedIndex::cmp_list_node(*it_pre, *it))
            {
                existsed_list.list.insert(it, *it_pre);
                ++it_pre;
            }
        }

        while (it_pre != pre_proc_list.list.end())
        {
            existsed_list.list.emplace_back(*it_pre);
            ++it_pre;
        }

        existsed_list.page_set.insert(id);

        // 创建新的倒排索引文件
        std::string opath = gen_invIndex_filepath(id);
        std::ofstream fout(opath, std::ios::out);
        boost::archive::binary_oarchive oa(fout);
        oa &existsed_list;
        fout.close();

        // 在mysql中更新值
        std::string sql = "UPDATE InvertedIndexTable SET path='" + opath + "'WHERE key='" + key + "';";
        while (true)
        {
            mysql_conn = db->mysql_conn_pool->get_conn(mysql_id);
            if (mysql_conn != NULL)
                break;
            else
                usleep(100); // 获取不到mysql conn， 100ms后重试
        }

        mysql_autocommit(mysql_conn, OFF);
        if (mysql_query(mysql_conn, sql.c_str()))
        {
            log->error(__LINE__, "mysql query failed.");
            mysql_rollback(mysql_conn);
            mysql_autocommit(mysql_conn, ON);
            db->mysql_conn_pool->free_conn(mysql_id);
            return;
        }
    }
    else // 之前不存在这个key的倒排索引
    {
        // 创建新的倒排索引文件
        std::string opath = gen_invIndex_filepath(id);
        std::ofstream fout(opath, std::ios::out);
        boost::archive::binary_oarchive oa(fout);
        oa &pre_proc_list;
        fout.close();

        // 在mysql中更新值
        std::string sql = "INSERT INTO InvertedIndexTable(key, path) VALUES('" + key + "', '" + opath + "');";

        while (true)
        {
            mysql_conn = db->mysql_conn_pool->get_conn(mysql_id);
            if (mysql_conn != NULL)
                break;
            else
                usleep(100); // 获取不到mysql conn， 100ms后重试
        }

        if (mysql_query(mysql_conn, sql.c_str()))
        {
            log->error(__LINE__, "mysql query failed.");
            db->mysql_conn_pool->free_conn(mysql_id);
            return;
        }

        // db->mysql_conn_pool->free_conn(mysql_id);
    }

    // 填写完全WebPage表
    std::string document = "", title = "";
    bj::array document_arr = msg_obj.at("content").as_array();
    bj::array title_arr = msg_obj.at("title").as_array();

    // 获取document的全部内容
    for (auto &x : document_arr)
        document += (bj::value_to<std::string>(x));

    // 获取title字符串
    for (auto &x : title_arr)
        title += (bj::value_to<std::string>(x));

    std::string sql = "UPDATE WebPage SET document='" + document + "', title='" + title + "' WHERE idWebPage=" + boost::lexical_cast<std::string>(id) + ";";
    mysql_commit(mysql_conn);
    mysql_autocommit(mysql_conn, ON);
    db->mysql_conn_pool->free_conn(mysql_id);

    // todo: 定时删除过期的倒排索引
}

/**
 * @brief 获取倒排索引文件在磁盘上的路径
 *
 * @return std::string
 */
static std::string get_inv_index_path(const std::string &key, logging::logger *log, MYSQL *mysql_conn)
{
    std::string sql;
    sql = "SELECT path FROM InvertedIndexTable WHERE key='" + key + "';";
    if (mysql_query(mysql_conn, sql.c_str()))
    {
        log->error(__LINE__, "mysql query failed.");
        return "-1";
    }

    MYSQL_RES *res;

    res = mysql_store_result(mysql_conn);
    assert(res != NULL);

    MYSQL_ROW column;

    if (mysql_affected_rows(mysql_conn) != 1)
        return "-1";

    while (column = mysql_fetch_row(res))
    {
        return boost::lexical_cast<std::string>(column[0]);
    }
    return "-1";
}

/**
 * @brief 生成倒排索引文件的k路径
 *
 * @param id 倒排索引的id
 * @return std::string 生成的路径
 */
std::string indexBuilder::gen_invIndex_filepath(const int &id)
{
    time_t t;
    time(&t);
    return indexBuilder::invIndex_file_base_path + boost::lexical_cast<std::string>(id) + '_' + boost::lexical_cast<std::string>(t) + ".inv_idx";
}

unsigned int indexBuilder::get_indexBuilder_id()
{
    unsigned int ret;
    mtx_indexBuilder_id.lock();

    ret = max_indexBuilder_id++;

    mtx_indexBuilder_id.unlock();
    return ret;
}