#include <iostream>
#include <database/DataBase.h>
#include <utils/configParser/configParser.h>
#include <utils/logger/logger.h>
#include <libs/kafka/KafkaProducer.h>
#include <boost/program_options.hpp>
#include <map>

namespace bpo = boost::program_options;

logging::logger log_main("main");

/**
 * 运行所有服务
 */
void do_run_all()
{
    std::cout << "Run all." << std::endl;
}

void do_init_mysql()
{
    configParser::parse("config.ini");
    Database::DataBase db;

    Database::init_mysql(db);
}

void do_test_kafka()
{
    using namespace kafka::clients;
    std::cout << "Test kafka." << std::endl;
    std::string brokers = "192.168.5.215:9092";
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

        for (auto line = std::make_shared<std::string>();
             std::getline(std::cin, *line);
             line = std::make_shared<std::string>()) {
            // The ProducerRecord doesn't own `line`, it is just a thin wrapper
            auto record = producer::ProducerRecord(topic,
                                                   kafka::NullKey,
                                                   kafka::Value(line->c_str(), line->size()));

            // Send the message
            producer.send(record,
                          // The delivery report handler
                          // Note: Here we capture the shared_pointer of `line`,
                          //       which holds the content for `record.value()`.
                          //       It makes sure the memory block is valid until the lambda finishes.
                          [line](const producer::RecordMetadata& metadata, const kafka::Error& error) {
                              if (!error) {
                                  std::cout << "% Message delivered: " << metadata.toString() << std::endl;
                              } else {
                                  std::cerr << "% Message delivery failed: " << error.message() << std::endl;
                              }
                          });

            if (line->empty()) break;
        }

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

    return 0;
}
