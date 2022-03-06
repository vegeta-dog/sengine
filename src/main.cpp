#include <iostream>
#include<database/DataBase.h>
#include<utils/configParser/configParser.h>
#include<utils/logger/logger.h>

#include<map>
logging::logger log_main("main");
int main() {

    Database::example();

    log_main.info(__LINE__, "sEngine starting...");

    configParser::parse("config.ini");
    Database::DataBase db;

    return 0;
}
