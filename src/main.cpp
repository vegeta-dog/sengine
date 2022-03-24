#include <iostream>
#include "database/DataBase.h"
#include "evaluator/evaluator.h"
#include "utils/configParser/configParser.h"
#include "utils/logger/logger.h"
#include "libs/kafka/KafkaProducer.h"
#include "libs/kafka/KafkaConsumer.h"
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <map>
#include <list>

namespace bpo = boost::program_options;

logging::logger log_main("main");
static std::list<boost::thread*> threads;
/**
 * 运行所有服务
 */
void do_run_all()
{
    std::cout << "Run all." << std::endl;
    boost::thread *t;
    t = new boost::thread(&Evaluator::run);
    threads.emplace_back(t);

    for (auto itr = threads.begin(); itr != threads.end(); ++itr)
    {
        (*itr)->join();
    }
}

void do_init_mysql()
{
    configParser::parse("config.ini");
    Database::DataBase db;

    Database::init_mysql(db);
}

void kafka_producer()
{
    configParser::parse("config.ini");
    using namespace kafka::clients;
    std::cout << "Test kafka." << std::endl;
    std::string brokers;
    configParser::get_config("Kafka.kafka_brokers", &brokers);
    kafka::Topic topic = "tp";

    // Create configuration object
    kafka::Properties props({
        {"bootstrap.servers", brokers},
        {"enable.idempotence", "true"},
    });

    // Create a producer instance.
    kafka::clients::KafkaProducer producer(props);

    // Read messages from stdin and produce to the broker
    std::cout << "% Type message value and hit enter to produce message. (empty line to quit)" << std::endl;

    int count = 0;
    while (count < 10)
    {
        std::shared_ptr<std::string> str = std::make_shared<std::string>();
        *str = boost::lexical_cast<std::string>(count);
        // The ProducerRecord doesn't own `line`, it is just a thin wrapper
        auto record = producer::ProducerRecord(topic,
                                               kafka::NullKey,
                                               kafka::Value(str->c_str(), str->size()));

        // Send the message
        producer.send(record,
                      // The delivery report handler
                      // Note: Here we capture the shared_pointer of `line`,
                      //       which holds the content for `record.value()`.
                      //       It makes sure the memory block is valid until the lambda finishes.
                      [str](const producer::RecordMetadata &metadata, const kafka::Error &error)
                      {
                          if (!error)
                          {
                              std::cout << "% Message delivered: " << metadata.toString() << std::endl;
                          }
                          else
                          {
                              std::cerr << "% Message delivery failed: " << error.message() << std::endl;
                          }
                      });
        ++count;
        boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
}

int kafka_consumer()
{
    configParser::parse("config.ini");

    std::string brokers;
    configParser::get_config("Kafka.kafka_brokers", &brokers);
    kafka::Topic topic = "tp";

    // Create configuration object
    kafka::Properties props({
        {"bootstrap.servers", brokers},
        {"enable.idempotence", "true"},
    });
    try
    {

        // Create configuration object
        kafka::Properties props({{"bootstrap.servers", brokers},
                                 {"enable.auto.commit", "true"}});

        // Create a consumer instance
        kafka::clients::KafkaConsumer consumer(props);

        // Subscribe to topics
        consumer.subscribe({topic});

        // Read messages from the topic
        std::cout << "% Reading messages from topic: " << topic << std::endl;
        while (true)
        {
            auto records = consumer.poll(std::chrono::milliseconds(100));
            for (const auto &record : records)
            {
                // In this example, quit on empty message
                if (record.value().size() == 0)
                    return 0;

                if (!record.error())
                {
                    std::cout << "% Got a new message..." << std::endl;
                    std::cout << "    Topic    : " << record.topic() << std::endl;
                    std::cout << "    Partition: " << record.partition() << std::endl;
                    std::cout << "    Offset   : " << record.offset() << std::endl;
                    std::cout << "    Timestamp: " << record.timestamp().toString() << std::endl;
                    std::cout << "    Headers  : " << kafka::toString(record.headers()) << std::endl;
                    std::cout << "    Key   [" << record.key().toString() << "]" << std::endl;
                    std::cout << "    Value [" << record.value().toString() << "]" << std::endl;
                }
                else
                {
                    std::cerr << record.toString() << std::endl;
                }
            }
        }

        // consumer.close(); // No explicit close is needed, RAII will take care of it
    }
    catch (const kafka::KafkaException &e)
    {
        std::cerr << "% Unexpected exception caught: " << e.what() << std::endl;
    }
    return 0;
}

void do_test_kafka()
{
    boost::thread p(&kafka_producer);
    boost::thread c(&kafka_consumer);
    p.join();
    c.join();
}

int main(int argc, char const *argv[])
{

    log_main.info(__LINE__, "sEngine starting...");

    // 构造选项描述器和选项存储器
    bpo::options_description opts("all options");
    bpo::variables_map vm;

    // 为选项描述器增加选项
    // 参数依次为key value的类型以及描述
    opts.add_options()("help", "帮助文档")("run-all", "运行全部服务")("init-mysql", "初始化MySQL数据库")("test-kafka", "测试kafka");

    // 解析命令行参数
    bpo::store(bpo::parse_command_line(argc, argv, opts), vm);
    // 通知vm容器发生变更
    bpo::notify(vm);

    // 没有指定参数则直接运行所有服务
    if (vm.empty())
        do_run_all();
    if (vm.count("help"))
    {
        std::cout << opts << std::endl;
        exit(0);
    }
    if (vm.count("init-mysql"))
    {
        do_init_mysql();
        exit(0);
    }

    if (vm.count("test-kafka"))
    {
        do_test_kafka();
        exit(0);
    }
    if (vm.count("run-all"))
    {
        do_run_all();
        exit(0);
    }

    return 0;
}
