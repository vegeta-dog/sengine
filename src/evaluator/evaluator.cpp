//
// Created by longjin on 2022/3/23.
//

#include "evaluator.h"

#include "../utils/configParser/configParser.h"
#include "../utils/queue/queue.h"

#include <boost/lockfree/queue.hpp>
#include <boost/lexical_cast.hpp>
#include <mutex>

#include <iostream>
static logging::logger log_evaluator("Evaluator");
static ThreadSafeQueue::queue<std::string> msg_queue;

/**
 * 启动evaluator模块
 */
void Evaluator::run()
{
    log_evaluator.info(__LINE__, "Evaluator starting...");
    configParser::parse("config.ini");
    Database::DataBase db;
}

void Evaluator::message_handler(kafka::clients::consumer::ConsumerRecord rec)
{
    msg_queue.push(rec.value().toString());
}

Evaluator::evaluator::evaluator(MYSQL *mysql_conn, unsigned int mysql_conn_id, redisContext *redis_conn, unsigned int redis_conn_id, Database::DataBase *db)
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

void Evaluator::evaluator::run()
{
}

unsigned int Evaluator::get_evaluator_id()
{
    unsigned int ret;
    mtx_evaluator_id.lock();

    ret = max_evaluator_id++;

    mtx_evaluator_id.unlock();
    return ret;
}