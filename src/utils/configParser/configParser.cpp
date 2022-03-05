//
// Created by longjin on 2022/3/5.
//

#include "configParser.h"
#include<cstdio>
#include<cstdlib>
#include<stdexcept>
#include<iostream>

int configParser::parse(const std::string& path, std::map<std::string, std::string> *conf_str, std::map<std::string, int> *conf_int,
          std::map<std::string, double> *conf_double)
{
    FILE *fp;
    if((fp = fopen(path.c_str(), "r+"))== nullptr)
    {
        return PATH_ERROR;
    }
    try {

    }
    catch (std::exception e)
    {
        printf("");
    }
    return SUCCESS;
};
