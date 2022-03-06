#include <iostream>
#include<database/DataBase.h>
#include<utils/configParser/configParser.h>
#include<utils/logger/logger.h>

#include<map>

int main() {

    Database::example();
    logging::logger logger("main");
    logger.info(__LINE__, "12123123");
    configParser::parse("config.ini");

    return 0;
}
