#include "indexBuilder.h"

#include "../utils/configParser/configParser.h"
#include "../utils/queue/queue.h"
#include "../utils/kafka/kafka_client.h"

#include <boost/lockfree/queue.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/json.hpp>

static logging::logger log_evaluator("IndexBuilder");

static std::list<boost::thread *> threads;
static std::list<indexBuilder::builder *> builder_objs;

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
    
}