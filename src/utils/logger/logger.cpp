//
// Created by longjin on 2022/3/6.
//

#include "logger.h"
#include "cstdlib"


logging::logger::logger(const std::string &module) {

    sprintf(this->module_name, "%s", module.c_str());
    this->pid = getpid();

}

void logging::logger::info(const int &line, const std::string &msg) {
    char buf[256];
    sprintf(buf, "[process: %d][module: %s][line:%d] %s ", this->pid, this->module_name, line, msg.c_str());
    spdlog::info(buf);
}

void logging::logger::debug(const int &line, const std::string &msg) {
    char buf[256];
    sprintf(buf, "[process: %d][module: %s][line:%d] %s ", this->pid, this->module_name, line, msg.c_str());
    spdlog::debug(buf);
}

void logging::logger::warn(const int &line, const std::string &msg) {
    char buf[256];
    sprintf(buf, "[process: %d][module: %s][line:%d] %s ", this->pid, this->module_name, line, msg.c_str());
    spdlog::warn(buf);

}

void logging::logger::error(const int &line, const std::string &msg) {
    char buf[256];
    sprintf(buf, "[process: %d][module: %s][line:%d] %s ", this->pid, this->module_name, line, msg.c_str());
    spdlog::error(buf);

}

void logging::logger::critical(const int &line, const std::string &msg) {
    char buf[256];
    sprintf(buf, "[process: %d][module: %s][line:%d] %s ", this->pid, this->module_name, line, msg.c_str());
    spdlog::critical(buf);
}

