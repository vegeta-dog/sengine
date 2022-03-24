#ifndef _KAFKA_CLIENT_H_
#define _KAFKA_CLIENT_H_

#include "../../utils/logger/logger.h"
#include "../../libs/kafka/KafkaProducer.h"
#include "../../libs/kafka/KafkaConsumer.h"
#include <string>
#include <mutex>

namespace Kafka_cli
{

    static unsigned int get_consumer_id(); // 获取consumer id的函数
    static std::mutex mtx_comsumer_id;     // 消费者id的锁
    static unsigned int max_consumer_id = 0;

    static unsigned int get_producer_id(); // 获取producer id的函数
    static std::mutex mtx_producer_id;
    static unsigned int max_producer_id = 0;

    class producer
    {
    public:
        producer(std::string brokers, std::string topic);
        ~producer();

        /**
         * @brief send message
         * 
         * @param msg message
         */
        void send(const std::string & msg);

    private:
        std::string brokers;
        kafka::Topic topic;
        kafka::clients::KafkaProducer *pdc;
        logging::logger *log;
        unsigned int producer_obj_id;
    };

    class consumer
    {
    public:
        /**
         * @brief Construct a new kafka consumer object
         *
         * @param brokers broker domain
         * @param topic topic name
         * @param auto_commit auto commit?
         * @param handler 数据上半部处理函数
         */
        consumer(std::string brokers, std::string topic, bool auto_commit, void (*handler)(kafka::clients::consumer::ConsumerRecord));
        ~consumer();
        void worker();

    private:
        std::string brokers;
        kafka::Topic topic;
        kafka::clients::KafkaConsumer *csm;
        logging::logger *log;
        unsigned int consumer_obj_id;

        // 消息处理函数
        void (*handler)(kafka::clients::consumer::ConsumerRecord);
    };
};
#endif