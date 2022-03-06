#include <iostream>
#include<database/DataBase.h>
#include<utils/configParser/configParser.h>
#include<map>

int main() {

    Database::example();

    configParser::parse("config.ini");
    std::string data;
    configParser::get_config("string.a", &data);
    std::cout<<data<<std::endl;
    return 0;
}
