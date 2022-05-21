//
// Created by longjin on 2022/3/23.
//

#include "evaluator.h"

#include <unistd.h>

#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <mutex>
#include <sstream>

#include "../utils/configParser/configParser.h"
#include "../utils/kafka-cpp/kafka_client.h"
#include "../utils/queue/queue.h"

namespace bj = boost::json;

static logging::logger log_evaluator("Evaluator");
static ThreadSafeQueue::queue<std::string>
    msg_queue;  // 从爬虫模块收到的信息的队列, msg from word_split!

static ThreadSafeQueue::queue<std::string>
    send2indexBuilder_queue;  // 发送消息到索引构建器的队列
static ThreadSafeQueue::queue<std::string>
    send2Crawler_queue;  // 发送消息到爬虫的队列

static std::list<boost::thread *> threads;
static std::list<Evaluator::evaluator *> eva_objs;

/**
 * 启动evaluator模块
 */
void Evaluator::run() {
  log_evaluator.info(__LINE__, "Evaluator starting...");
  configParser::parse("config.ini");
  Database::DataBase db;

  std::string str;
  configParser::get_config("Evaluator.evaluator_num", &str);
  unsigned int evaluator_num = boost::lexical_cast<int>(str);

  log_evaluator.info(__LINE__,
                     "-----evaluator_num : " + std::to_string(evaluator_num));

  boost::thread *t;
  Evaluator::evaluator *eva_ptr;

  for (unsigned int i = 0; i < evaluator_num; ++i) {
    int mysql_id, redis_id;
    MYSQL *mysql_conn = db.mysql_conn_pool->get_conn(mysql_id);
    redisContext *redis_conn = db.redis_conn_pool->get_conn(redis_id);

    t = new boost::thread(&Evaluator::do_start, mysql_conn, mysql_id,
                          redis_conn, redis_id, &db);
    threads.emplace_back(t);
  }

  // 启动kafka客户端
  configParser::get_config("Kafka.kafka_brokers", &str);
  t = new boost::thread(&Kafka_cli::do_start_kafka_consumer, str,
                        "Crawler2Evaluator", "Eva_recv_crawl",
                        &Evaluator::message_handler);
  threads.emplace_back(t);

  t = new boost::thread(&Kafka_cli::do_start_kafka_producer, str,
                        "Evaluator2indexBuilder",
                        &Evaluator::send_msg2indexBuilder_handler);
  threads.emplace_back(t);

  t = new boost::thread(&Kafka_cli::do_start_kafka_producer, str,
                        "Evaluator2Crawler",
                        &Evaluator::send_msg2Crawler_handler);
  threads.emplace_back(t);

  for (auto x = threads.begin(); x != threads.end(); ++x) (*x)->join();
}

/**
 * @brief
 *
 * @return std::string
 */
std::string Evaluator::send_msg2indexBuilder_handler() {
  while (true) {
    try {
      return send2indexBuilder_queue.getFront();
    } catch (int e) {
      if (e == -1)
        usleep(100);
      else
        std::cerr << "Evaluator.cpp : At line " << __LINE__
                  << ": unexpected exception" << std::endl;
    }
  }
}

std::string Evaluator::send_msg2Crawler_handler() {
  while (true) {
    try {
      return send2Crawler_queue.getFront();
    } catch (int e) {
      if (e == -1)
        usleep(100);
      else
        std::cerr << "Evaluator.cpp : At line " << __LINE__
                  << ": unexpected exception" << std::endl;
    }
  }
}

/**
 * @brief 真正启动评估器
 *
 */
void Evaluator::do_start(MYSQL *mysql_conn, int mysql_conn_id,
                         redisContext *redis_conn, int redis_conn_id,
                         Database::DataBase *db) {
  Evaluator::evaluator eva(mysql_conn, mysql_conn_id, redis_conn, redis_conn_id,
                           db);
  eva_objs.emplace_back(&eva);
  eva.run();
}

void Evaluator::message_handler(kafka::clients::consumer::ConsumerRecord rec) {
  msg_queue.push(rec.value().toString());
}

Evaluator::evaluator::evaluator(MYSQL *mysql_conn, int mysql_conn_id,
                                redisContext *redis_conn, int redis_conn_id,
                                Database::DataBase *db) {
  this->id = get_evaluator_id();
  this->log = new logging::logger("Evaluator " +
                                  boost::lexical_cast<std::string>(this->id));

  this->mysql_conn = mysql_conn;
  this->mysql_conn_id = mysql_conn_id;

  this->redis_conn = redis_conn;
  this->redis_conn_id = redis_conn_id;

  this->db = db;
}

Evaluator::evaluator::~evaluator() {
  free(this->log);

  this->db->mysql_conn_pool->free_conn(mysql_conn_id);
  this->db->redis_conn_pool->free_conn(redis_conn_id);
}

/**
 * @brief 检查url是否在数据库内
 *
 * @param url
 * @return true
 * @return false
 */
bool Evaluator::evaluator::check_url_in_db(const std::string &url) {
  std::string key;
  // 先检查是否在redis内
  key = "WebPage:" + url;
  redisReply *reply =
      (redisReply *)redisCommand(this->redis_conn, ("GET " + key).c_str());
  if (reply != NULL && reply->type == REDIS_REPLY_STATUS) {
    if (reply->str == "1") {
      freeReplyObject(reply);
      return true;
    }
    printf("%s\n", reply->str);
  }
  freeReplyObject(reply);

  // redis中不存在，查库

  std::string sql;
  sql = "SELECT idWebPage, Crawl, UpdatedTime FROM WebPage WHERE url='" + url +
        "';";

  // std::cout << mysql_query(this->mysql_conn, sql.c_str()) << std::endl;

  if (mysql_query(this->mysql_conn, sql.c_str())) {
    log->error(__LINE__, "mysql query failed. Message:" +
                             boost::lexical_cast<std::string>(
                                 mysql_error(this->mysql_conn)));
    return false;
  }

  MYSQL_RES *res;

  res = mysql_store_result(this->mysql_conn);
  assert(res != NULL);

  MYSQL_ROW column;
  std::string _updatedTime = "";
  bool _crawl;
  std::string _id;

  if (mysql_affected_rows(this->mysql_conn) == 0) return false;
  // assert(mysql_affected_rows(this->mysql_conn) == 1);
  while (column = mysql_fetch_row(res)) {
    _id = column[0];
    _crawl = boost::lexical_cast<int>(column[1]);
    _updatedTime = boost::lexical_cast<std::string>(column[2]);
  }

  log->debug(__LINE__, "mysql query");

  mysql_free_result(res);

  // 数据库中不存在
  if (_updatedTime == "") return false;

  // 将url到id的映射存入redis
  key = "WebPage:" + url;
  reply = (redisReply *)redisCommand(this->redis_conn,
                                     ("SET " + key + " " + _id).c_str());
  if (reply == NULL) {
    log->error(__LINE__, "Redis reply is NULL!");
    freeReplyObject(reply);
    return false;
  }
  reply = (redisReply *)redisCommand(this->redis_conn,
                                     ("EXPIRE " + key + " 86400").c_str());
  freeReplyObject(reply);

  key = " WebPage:" + url + ":UpdatedTime ";
  reply = (redisReply *)redisCommand(this->redis_conn,
                                     ("SET" + key + _updatedTime).c_str());
  if (reply == NULL) {
    log->error(__LINE__, "Redis reply is NULL!");
    freeReplyObject(reply);
    return false;
  }
  reply = (redisReply *)redisCommand(this->redis_conn,
                                     ("EXPIRE " + key + " 86400").c_str());
  freeReplyObject(reply);

  key = " WebPage:" + url + ":Crawl ";
  reply = (redisReply *)redisCommand(
      this->redis_conn,
      ("SET" + key + boost::lexical_cast<std::string>(int(_crawl))).c_str());
  if (reply == NULL) {
    log->error(__LINE__, "Redis reply is NULL!");
    freeReplyObject(reply);
    return false;
  }
  reply = (redisReply *)redisCommand(this->redis_conn,
                                     ("EXPIRE " + key + " 86400").c_str());
  freeReplyObject(reply);

  return true;
}

/**
 * @brief 将网页链接存入数据库
 *  若网页链接不在数据库中，则存入数据库，否则直接返回链接的id
 * @param url
 * @return 主键ID
 */
int Evaluator::evaluator::store_weblink2db(const std::string &url,
                                           unsigned int crawl) {
  int ret = 0;
  log->info(__LINE__, "store_weblink2db");
  if (!Evaluator::evaluator::check_url_in_db(url)) {
    // url在数据库中不存在
    log->info(__LINE__, "url在数据库中不存在 insert");
    char sql[2048];
    sprintf(
        sql,
        "INSERT INTO WebPage(idWebPage, url, Crawl)  VALUES(NULL, '%s', %d)",
        url.c_str(), crawl);
    mysql_autocommit(this->mysql_conn, OFF);
    try {
      mysql_query(this->mysql_conn, sql);
      if (mysql_commit(this->mysql_conn)) {
        log->error(__LINE__, "mysql query failed. Message:" +
                                 boost::lexical_cast<std::string>(
                                     mysql_error(this->mysql_conn)));
        ret = -1;
        mysql_rollback(this->mysql_conn);
      } else
        ret = mysql_insert_id(this->mysql_conn);
    } catch (const std::exception &e) {
      mysql_rollback(this->mysql_conn);
      log->error(__LINE__, "mysql query failed. Message:" +
                               boost::lexical_cast<std::string>(e.what()));
    }

    mysql_autocommit(this->mysql_conn, ON);
  } 
  else  // url 在数据库中已存在
  {

    // todo: 将已爬取的网页的Crawl设置为1

    // 先在redis中查询idWebPage
    log->info(__LINE__, "url在数据库中已经存在 update");
    std::string key = "WebPage:" + url;

    redisReply *reply =
        (redisReply *)redisCommand(this->redis_conn, ("GET " + key).c_str());
    if (reply == NULL) {
      log->error(__LINE__, "Redis reply is NULL!");
      freeReplyObject(reply);
    } else
      return boost::lexical_cast<int>(reply->str);  // 在redis中找到数据

    std::string sql;
    sql = "SELECT idWebPage FROM WebPage WHERE url='" + url + "';";
    if (mysql_query(this->mysql_conn, sql.c_str())) {
      log->error(__LINE__, "mysql query failed.");
      return false;
    }

    MYSQL_RES *res;

    res = mysql_store_result(this->mysql_conn);
    assert(res != NULL);

    MYSQL_ROW column;

    assert(mysql_affected_rows(this->mysql_conn) == 1);
    while (column = mysql_fetch_row(res)) {
      ret = boost::lexical_cast<int>(column[0]);
    }

    // 将url到id的映射存入redis
    key = "WebPage:" + url;
    reply = (redisReply *)redisCommand(
        this->redis_conn,
        ("SET " + key + " " + boost::lexical_cast<std::string>(ret)).c_str());
    if (reply == NULL) {
      log->error(__LINE__, "Redis reply is NULL!");
      freeReplyObject(reply);
    }
    reply = (redisReply *)redisCommand(this->redis_conn,
                                       ("EXPIRE " + key + " 86400").c_str());
    freeReplyObject(reply);
  }
  return ret;
}
/**
 * @brief 评估器评估函数
 *  初次迭代中，暂不对内容进行评估，只是将网页数据往索引构建器传递
 */
void Evaluator::evaluator::run() {
  log->info(__LINE__, "Evaluator started.");
  while (true) {
    try {
      // 解析json
      std::string msg;
      try {
        msg = msg_queue.getFront();
      } catch (int e) {
        if (e == -1)
          usleep(100);  // can't get msg !!!!!
        else
          std::cerr << "At line " << __LINE__ << ": unexpected exception"
                    << std::endl;
        continue;
      }
      std::cout << "very begin" << std::endl;

      bj::value jv = bj::parse(msg);
      log->info(__LINE__, "tthh");
      auto msg_obj = jv.as_object();
      log->info(__LINE__, "tt");
      std::string url = bj::value_to<std::string>(msg_obj.at("url"));
      log->info(__LINE__, "gg");
      bj::value url_list = msg_obj.at("url_list");
      log->info(__LINE__, "1212121");
      int from_page_id = store_weblink2db(url, 1);

      std::cout << "fuck 1" << std::endl;

      auto urls = url_list.as_array();
      int to_page_id;
      for (const auto &u : urls) {
        std::cout << "very before" << std::endl;
        std::string x = bj::value_to<std::string>(u);
        // 将网页上带有的链接存入数据库

        log->warn(__LINE__, "urlx = " + x);

        to_page_id = store_weblink2db(x, 0);

        if (!to_page_id) {
            log->info(__LINE__, "str to_page_id = " + x);
        }

        // 创建weblink
        create_LinkRecord(from_page_id, to_page_id);

        // 暂时把所有的url都发回爬虫
        std::cout << "before" << std::endl;
        boost::json::object to_crawler;
        to_crawler["url"] = x;

        send2Crawler_queue.push(boost::json::serialize(to_crawler));
        std::cout << "after" << std::endl;
      }

      std::cout << "fuck you 2" << std::endl;

      // 填写count_total_link_to
      std::string sql =
          "UPDATE WebPage SET count_total_link_to=" +
          boost::lexical_cast<std::string>(urls.size()) +
          " WHERE idWebPage=" + boost::lexical_cast<std::string>(from_page_id) +
          ";";
      mysql_autocommit(this->mysql_conn, OFF);

      if (mysql_query(this->mysql_conn, sql.c_str())) {
        this->log->error(__LINE__, "mysql query failed.");
        mysql_rollback(this->mysql_conn);
        mysql_autocommit(this->mysql_conn, ON);
        throw "mysql query failed.";
      }
      mysql_commit(this->mysql_conn);

      mysql_autocommit(this->mysql_conn, ON);

      // 暂时所有网页都收录
      // todo: 评估内容
      boost::json::object to_index_builder;
      to_index_builder["id"] = boost::lexical_cast<uint64_t>(from_page_id);
      to_index_builder["title"] = msg_obj.at("title");
      to_index_builder["content"] = msg_obj.at("content");

      // 将下一步要爬取的链接发回给爬虫模块
      // 不是给爬虫，这里应该是给索引构建器
      send2indexBuilder_queue.push(boost::json::serialize(to_index_builder));
      log->info(__LINE__, "push ok!");
    } catch (const std::exception &e) {
      log->error(__LINE__, boost::lexical_cast<std::string>(e.what()));
    }
  }
}

/**
 * @brief 创建网页指向关系记录
 *
 * @param from  来源网页的id
 * @param to 被指向的网页的id
 * @return true 成功创建
 * @return false 创建失败
 */
bool Evaluator::evaluator::create_LinkRecord(unsigned int from,
                                             unsigned int to) {
  // 首先检查指向关系是否存在
  char sql[2048];

  sprintf(sql,
          "SELECT COUNT(*) FROM LinkTable WHERE LinkTable.From=%d AND "
          "LinkTable.To=%d;",
          from, to);

  if (mysql_query(this->mysql_conn, sql)) {
    log->error(__LINE__, "mysql query failed. Message:" +
                             boost::lexical_cast<std::string>(
                                 mysql_error(this->mysql_conn)));
    return false;
  }

  MYSQL_RES *res;

  res = mysql_store_result(this->mysql_conn);

  MYSQL_ROW column;

  // assert(mysql_affected_rows(this->mysql_conn) == 1);
  int count;
  while (column = mysql_fetch_row(res)) {
    count = boost::lexical_cast<int>(column[0]);
  }

  if (count > 0) return true;

  // 不存在链接，创建链接
  mysql_autocommit(this->mysql_conn, OFF);
  sprintf(sql,
          "INSERT INTO LinkTable(LinkTable.From, LinkTable.To) VALUES(%d, %d);",
          from, to);

  std::cout << "from = " << from << " , to = " << to << std::endl;

  if (mysql_query(this->mysql_conn, sql)) {
    log->error(__LINE__, "Failed to create LinkRecord. Message:" +
                             boost::lexical_cast<std::string>(
                                 mysql_error(this->mysql_conn)));
    mysql_rollback(this->mysql_conn);
    mysql_autocommit(this->mysql_conn, ON);
    return false;
  }

  mysql_autocommit(this->mysql_conn, ON);
  return true;
}

unsigned int Evaluator::get_evaluator_id() {
  unsigned int ret;
  mtx_evaluator_id.lock();

  ret = max_evaluator_id++;

  mtx_evaluator_id.unlock();
  return ret;
}