#include <iostream>
#include "database/DataBase.h"
#include "evaluator/evaluator.h"
#include "indexBuilder/indexBuilder.h"
#include "searcher/searcher.h"
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
static std::list<boost::thread *> threads;

void run_evaluator()
{
    log_main.info(__LINE__, "Starting evaluator...");
    boost::thread *t;
    t = new boost::thread(&Evaluator::run);
    threads.emplace_back(t);
}
void run_indexBuilder()
{
    log_main.info(__LINE__, "Starting index builder...");
    boost::thread *t;
    t = new boost::thread(&indexBuilder::run);
    threads.emplace_back(t);
}

void run_searcher()
{
    log_main.info(__LINE__, "Starting searcher...");
    boost::thread *t;
    t = new boost::thread(&Searcher::run);
    threads.emplace_back(t);
}

/**
 * 运行所有服务
 */
void do_run_all()
{
    run_evaluator();
    //run_indexBuilder();
    run_searcher();

}

void do_init_mysql()
{
    configParser::parse("config.ini");
    Database::DataBase db;

    Database::init_mysql(db);
}

int main(int argc, char const *argv[])
{

    log_main.info(__LINE__, "sEngine starting...");

    // 构造选项描述器和选项存储器
    bpo::options_description opts("all options");
    bpo::variables_map vm;

    // 为选项描述器增加选项
    // 参数依次为key value的类型以及描述
    opts.add_options()
        ("help", "帮助文档")
        ("run-all", "运行全部服务")
        ("run-evaluator", "运行内容评估器")
        ("run-index-builder", "运行索引构建器")
        ("run-searcher", "运行内容检索器")
        ("init-mysql", "初始化MySQL数据库");

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
    }
    else if (vm.count("init-mysql"))
        do_init_mysql();

    else if (vm.count("run-all"))
        do_run_all();
    else if(vm.count("run-evaluator"))
        run_evaluator();
    else if(vm.count("run-index-builder"))
        run_indexBuilder();
    else if(vm.count("run-searcher"))
        run_searcher();
    
    for (auto itr = threads.begin(); itr != threads.end(); ++itr)
    {
        (*itr)->join();
    }

    return 0;
}
