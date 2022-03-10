#include <iostream>
#include<database/DataBase.h>
#include<utils/configParser/configParser.h>
#include<utils/logger/logger.h>
#include<boost/program_options.hpp>
#include<map>

namespace bpo = boost::program_options;

logging::logger log_main("main");

/**
 * 运行所有服务
 */
void do_run_all()
{
    std::cout<<"Run all."<<std::endl;
}

void do_init_mysql()
{
    configParser::parse("config.ini");
    Database::DataBase db;

    Database::init_mysql(db);

}
int main(int argc, char const *argv[]) {

    log_main.info(__LINE__, "sEngine starting...");

    // 构造选项描述器和选项存储器
    bpo::options_description opts("all options");
    bpo::variables_map vm;

    // 为选项描述器增加选项
    // 参数依次为key value的类型以及描述
    opts.add_options()
            ("help", "帮助文档")
            ("run-all", "运行全部服务")
            ("init-mysql", "初始化MySQL数据库");

    // 解析命令行参数
    bpo::store(bpo::parse_command_line(argc, argv, opts), vm);
    // 通知vm容器发生变更
    bpo::notify(vm);

    // 没有指定参数则直接运行所有服务
    if(vm.empty())
        do_run_all();
    if(vm.count("help"))
    {
        std::cout<<opts<<std::endl;
        exit(0);
    }
    if(vm.count("init-mysql"))
        do_init_mysql();



    return 0;
}
