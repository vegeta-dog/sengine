//
// Created by longjin on 2022/3/23.
//

#include "evaluator.h"

#include <unistd.h>

#include "../utils/configParser/configParser.h"
#include "../utils/queue/queue.h"
#include "../utils/kafka/kafka_client.h"

#include <boost/lockfree/queue.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/json.hpp>

#include <mutex>

#include <iostream>
#include <sstream>

namespace bj = boost::json;

static logging::logger log_evaluator("Evaluator");
static ThreadSafeQueue::queue<std::string> msg_queue;   // 从爬虫模块收到的信息的队列

static ThreadSafeQueue::queue<std::string> send2indexBuilder_queue; // 发送消息到索引构建器的队列

static std::list<boost::thread *> threads;
static std::list<Evaluator::evaluator *> eva_objs;



/**
 * 启动evaluator模块
 */
void Evaluator::run()
{
    log_evaluator.info(__LINE__, "Evaluator starting...");
    configParser::parse("config.ini");
    Database::DataBase db;

    std::string str;
    configParser::get_config("Evaluator.evaluator_num", &str);
    unsigned int evaluator_num = boost::lexical_cast<int>(str);

    boost::thread *t;
    Evaluator::evaluator *eva_ptr;

    for (unsigned int i = 0; i < evaluator_num; ++i)
    {
        int mysql_id, redis_id;
        MYSQL *mysql_conn = db.mysql_conn_pool->get_conn(mysql_id);
        redisContext *redis_conn = db.redis_conn_pool->get_conn(redis_id);

        t = new boost::thread(&Evaluator::do_start, mysql_conn, mysql_id, redis_conn, redis_id, &db);
        threads.emplace_back(t);
    }

    // 启动kafka客户端
    configParser::get_config("Kafka.kafka_brokers", &str);
    t = new boost::thread(&Kafka_cli::do_start_kafka_consumer, str, "Crawler2Evaluator", "Eva_recv_crawl", &Evaluator::message_handler);
    threads.emplace_back(t);
    
    t = new boost::thread(&Kafka_cli::do_start_kafka_producer, str, "Evaluator2indexBuilder", &Evaluator::send_msg2indexBuilder_handler);
    threads.emplace_back(t);

    for (auto x = threads.begin(); x != threads.end(); ++x)
        (*x)->join();
}

std::string Evaluator::send_msg2indexBuilder_handler()
{
    while (true)
    {

        try
        {
            return send2indexBuilder_queue.getFront();
        }
        catch (int e)
        {
            if (e == -1)
                usleep(100);
            else
                std::cerr << "At line " << __LINE__ << ": unexpected exception" << std::endl;
        }
    }
}
/**
 * @brief 真正启动评估器
 *
 */
void Evaluator::do_start(MYSQL *mysql_conn, int mysql_conn_id, redisContext *redis_conn, int redis_conn_id, Database::DataBase *db)
{
    Evaluator::evaluator eva(mysql_conn, mysql_conn_id, redis_conn, redis_conn_id, db);
    eva_objs.emplace_back(&eva);
    eva.run();
}

void Evaluator::message_handler(kafka::clients::consumer::ConsumerRecord rec)
{
    msg_queue.push(rec.value().toString());
}

Evaluator::evaluator::evaluator(MYSQL *mysql_conn, int mysql_conn_id, redisContext *redis_conn, int redis_conn_id, Database::DataBase *db)
{
    this->id = get_evaluator_id();
    this->log = new logging::logger("Evaluator " + boost::lexical_cast<std::string>(this->id));

    this->mysql_conn = mysql_conn;
    this->mysql_conn_id = mysql_conn_id;

    this->redis_conn = redis_conn;
    this->redis_conn_id = redis_conn_id;

    this->db = db;
}

Evaluator::evaluator::~evaluator()
{
    free(this->log);

    this->db->mysql_conn_pool->free_conn(mysql_conn_id);
    this->db->redis_conn_pool->free_conn(redis_conn_id);
}

/**
 * @brief 将网页链接存入数据库
 *
 * @param url
 */
void Evaluator::evaluator::store_weblink2db(const std::string &url)
{
    if (!Evaluator::evaluator::check_url_in_db(url))
    {
        // url在数据库中不存在
        char sql[2048];
        sprintf(sql, "INSERT INTO WebLink(idWebLink, url, Crawl)  VALUES(NULL, '%s', 0)", url.c_str());
        if (mysql_query(this->mysql_conn, sql))
        {
            log->error(__LINE__, "mysql query failed. Message:" + boost::lexical_cast<std::string>(mysql_error(this->mysql_conn)));
            return;
        }
    }
}
/**
 * @brief 评估器评估函数
 *  初次迭代中，暂不对内容进行评估，只是将网页数据往索引构建器传递
 */
void Evaluator::evaluator::run()
{
    log->info(__LINE__, "Evaluator started.");
    while (true)
    {
        try
        {

            // 解析json
            std::string msg;
            try
            {
                msg = msg_queue.getFront();
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
            auto msg_obj = jv.as_object();

            std::string url = bj::value_to<std::string>(msg_obj.at("url"));
            bj::value url_list = msg_obj.at("url_list");

            if (!check_url_in_db(url))
                store_weblink2db(url);

            auto urls = url_list.as_array();

            for (const auto &u : urls)
            {
                std::string x = bj::value_to<std::string>(u);

                // 将网页上带有的链接存入数据库
                store_weblink2db(x);
            }
            
            // 暂时所有网页都收录
            // todo: 评估内容
            send2indexBuilder_queue.push(msg);
            // todo: 将下一步要爬取的链接发回给爬虫模块
        }
        catch (const std::exception &e)
        {
            log->error(__LINE__, boost::lexical_cast<std::string>(e.what()));
        }
    }
}

/**
 * @brief 检查url是否在数据库内
 *
 * @param url
 * @return true
 * @return false
 */
bool Evaluator::evaluator::check_url_in_db(const std::string &url)
{
    std::string key;
    // 先检查是否在redis内
    key = "weblink:" + url;
    redisReply *reply = (redisReply *)redisCommand(this->redis_conn, ("GET " + key).c_str());
    if (reply != NULL && reply->type == REDIS_REPLY_STATUS)
    {
        if (reply->str == "1")
        {
            freeReplyObject(reply);
            return true;
        }
        printf("%s\n", reply->str);
    }
    freeReplyObject(reply);

    // redis中不存在，查库

    std::string sql;
    sql = "SELECT Crawl, UpdatedDateTime FROM WebLink WHERE url='" + url + "';";
    if (mysql_query(this->mysql_conn, sql.c_str()))
    {
        log->error(__LINE__, "mysql query failed.");
        return false;
    }

    MYSQL_RES *res;

    res = mysql_store_result(this->mysql_conn);
    assert(res != NULL);

    MYSQL_ROW column;
    std::string _updatedTime = "";
    bool _crawl;

    while (column = mysql_fetch_row(res))
    {
        _crawl = boost::lexical_cast<int>(column[0]);
        _updatedTime = boost::lexical_cast<std::string>(column[1]);
    }

    mysql_free_result(res);

    // 数据库中不存在
    if (_updatedTime == "")
        return false;

    key = "weblink:" + url;
    reply = (redisReply *)redisCommand(this->redis_conn, ("SET " + key + " 1").c_str());
    if (reply == NULL)
    {
        log->error(__LINE__, "Redis reply is NULL!");
        freeReplyObject(reply);
        return false;
    }
    reply = (redisReply *)redisCommand(this->redis_conn, ("EXPIRE " + key + " 86400").c_str());
    freeReplyObject(reply);

    key = " weblink:" + url + ":UpdatedDateTime ";
    reply = (redisReply *)redisCommand(this->redis_conn, ("SET" + key + _updatedTime).c_str());
    if (reply == NULL)
    {
        log->error(__LINE__, "Redis reply is NULL!");
        freeReplyObject(reply);
        return false;
    }
    reply = (redisReply *)redisCommand(this->redis_conn, ("EXPIRE " + key + " 86400").c_str());
    freeReplyObject(reply);

    key = " weblink:" + url + ":Crawl ";
    reply = (redisReply *)redisCommand(this->redis_conn, ("SET" + key + boost::lexical_cast<std::string>(int(_crawl))).c_str());
    if (reply == NULL)
    {
        log->error(__LINE__, "Redis reply is NULL!");
        freeReplyObject(reply);
        return false;
    }
    reply = (redisReply *)redisCommand(this->redis_conn, ("EXPIRE " + key + " 86400").c_str());
    freeReplyObject(reply);

    return true;
}

unsigned int Evaluator::get_evaluator_id()
{
    unsigned int ret;
    mtx_evaluator_id.lock();

    ret = max_evaluator_id++;

    mtx_evaluator_id.unlock();
    return ret;
}