#include "indexBuilder.h"

#include "../utils/configParser/configParser.h"
#include "../utils/queue/queue.h"
#include "../utils/kafka-cpp/kafka_client.h"

#include <boost/lockfree/queue.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/json.hpp>

namespace bj = boost::json;

static logging::logger log_evaluator("IndexBuilder");

static std::list<boost::thread *> threads;
static std::list<indexBuilder::builder *> builder_objs;

static ThreadSafeQueue::queue<std::string> recv_from_eva_queue;

indexBuilder::builder::builder(Database::DataBase *db)
{
    this->db = db;
}

indexBuilder::builder::~builder()
{
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
void do_start(Database::DataBase *db)
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

            // 异步io，分批次读取磁盘上的倒排列表到内存中，并进行合并

            

        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
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
    boost::json::array arr[0] = msg_obj.at("title").as_array();
    boost::json::array arr[1] = msg_obj.at("content").as_array();

    unsigned int id = msg_obj.at("id").as_uint64();

    unsigned int offset = 0;
    for (const auto &ar : arr)
        for (boost::json::value item : ar)
        {
            std::string str = bj::value_to<std::string>(item);

            if (!inv_map.count(str)) // 当前网页还没有统计过这个key，创建倒排列表
            {
                inv_map[str] = indexBuilder::InvertedIndex::InvertedIndex_List(str);
            }

            inv_map[str].list.emplace_back(indexBuilder::InvertedIndex::list_node(id, offset));

            offset += str.length();
        }
}