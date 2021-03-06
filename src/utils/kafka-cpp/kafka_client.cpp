#include "kafka_client.h"
#include <boost/lexical_cast.hpp>

// @todo: 限制队列大小 

Kafka_cli::consumer::consumer(std::string brokers, std::string topic, bool auto_commit, std::string group_id, void (*hdl)(kafka::clients::consumer::ConsumerRecord))
{
    this->brokers = brokers;
    this->topic = topic;
    this->handler = hdl;
    this->group_id = group_id;
    this->consumer_obj_id = get_consumer_id();

    this->log = new logging::logger("consumer " + boost::lexical_cast<std::string>(this->consumer_obj_id));

    // Create configuration object
    kafka::Properties props({
        {"bootstrap.servers", this->brokers},
        {"enable.auto.commit", (auto_commit ? "true" : "false")},
        {"group.id", this->group_id},
        {"auto.offset.reset", "earliest"},
    });

    this->csm = new kafka::clients::KafkaConsumer(props);
}

Kafka_cli::consumer::~consumer()
{
    this->csm->close();
    free(this->csm);
    free(this->log);
}

void Kafka_cli::consumer::worker()
{
    this->csm->subscribe({this->topic});

    while (true)
    {
        try
        {
            auto records = this->csm->poll(std::chrono::milliseconds(100));

            for (const auto &rec : records)
            {
                // 忽略空消息
                if (rec.value().size() == 0)
                    continue;
                if (!rec.error())
                    this->handler(rec);
                else
                    log->error(__LINE__, "rec_error:" + rec.toString());
            }
        }
        catch (const kafka::KafkaException &e)
        {
            log->error(__LINE__, "Unexpected exception caught: " + boost::lexical_cast<std::string>(e.what()));
        }
    }
}

unsigned int Kafka_cli::get_consumer_id()
{
    unsigned int ret;
    mtx_comsumer_id.lock();

    ret = max_consumer_id++;

    mtx_comsumer_id.unlock();
    return ret;
}

Kafka_cli::producer::producer(std::string brokers, std::string topic, std::string (*handler)(void))
{
    this->brokers = brokers;
    this->topic = topic;
    this->producer_obj_id = get_producer_id();

    this->handler = handler;

    this->log = new logging::logger("producer " + boost::lexical_cast<std::string>(this->producer_obj_id));

    // Create configuration object
    kafka::Properties props({
        {"bootstrap.servers", this->brokers},
        {"enable.idempotence", "true"},
    });

    this->pdc = new kafka::clients::KafkaProducer(props);
}

Kafka_cli::producer::~producer()
{
    this->pdc->close();
    free(this->pdc);
    free(this->log);
}

void Kafka_cli::producer::worker()
{
    while (true)
    {
        try
        {
            this->send(this->handler());
        }
        catch (const std::exception &e)
        {
            log->error(__LINE__, "An error occurred, Details:" + boost::lexical_cast<std::string>(e.what()));
        }
    }
}

void Kafka_cli::producer::send(const std::string &msg)
{
    try
    {
        auto record = kafka::clients::producer::ProducerRecord(this->topic, kafka::NullKey, kafka::Value(msg.c_str(), msg.size()));

        this->pdc->send(
            record, [](const kafka::clients::producer::RecordMetadata &metadata, const kafka::Error &error)
            {
                              if (!error) {
                                  std::cout << "% Message delivered: " << metadata.toString() << std::endl;
                              } else {
                                  std::cerr << "% Message delivery failed: " << error.message() << std::endl;
                              } },
            // The memory block given by record.value() would be copied
            kafka::clients::KafkaProducer::SendOption::ToCopyRecordValue);
    }
    catch (const kafka::KafkaException &e)
    {
        log->error(__LINE__, "Unexpected exception caught: " + boost::lexical_cast<std::string>(e.what()));
    }
}

unsigned int Kafka_cli::get_producer_id()
{
    unsigned int ret;
    mtx_producer_id.lock();

    ret = max_producer_id++;

    mtx_producer_id.unlock();
    return ret;
}

void Kafka_cli::do_start_kafka_consumer(const std::string &brokers, const std::string &topic, std::string group_id, void (*handler)(kafka::clients::consumer::ConsumerRecord))
{
    Kafka_cli::consumer csm(brokers, topic, true, group_id, handler);
    csm.worker();
}

void Kafka_cli::do_start_kafka_producer(const std::string &brokers, const std::string &topic, std::string (*handler)(void))
{
    Kafka_cli::producer pdc(brokers, topic, handler);
    pdc.worker();
}