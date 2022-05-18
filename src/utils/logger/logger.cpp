//
// Created by longjin on 2022/3/6.
//

#include "logger.h"
#include "cstdlib"
#include "unistd.h"

// #define BUF_SIZE (2048)
#define delta (64)
#define MASK_64 ((1 << 6) - 1)
#define ALIGN_64(x) ((x + 63) & (~MASK_64))

logging::logger::logger(const std::string &module)
{
    sprintf(this->module_name, "%s", module.c_str());
    this->pid = getpid();
}

void logging::logger::info(const int &line, const std::string &msg)
{
    char *buf = new char[ALIGN_64(msg.size() + delta)];
    sprintf(buf, "[process: %d][module: %s][line:%d] %s ", this->pid, this->module_name, line, msg.c_str());
    spdlog::info(buf);
    delete buf;
}

void logging::logger::debug(const int &line, const std::string &msg)
{
    char *buf = new char[ALIGN_64(msg.size() + delta)];
    sprintf(buf, "[process: %d][module: %s][line:%d] %s ", this->pid, this->module_name, line, msg.c_str());
    spdlog::debug(buf);
    delete buf;
}

void logging::logger::warn(const int &line, const std::string &msg)
{
    char *buf = new char[ALIGN_64(msg.size() + delta)];
    sprintf(buf, "[process: %d][module: %s][line:%d] %s ", this->pid, this->module_name, line, msg.c_str());
    spdlog::warn(buf);
    delete buf;
}

void logging::logger::error(const int &line, const std::string &msg)
{
    char *buf = new char[ALIGN_64(msg.size() + delta)];
    sprintf(buf, "[process: %d][module: %s][line:%d] %s ", this->pid, this->module_name, line, msg.c_str());
    spdlog::error(buf);
    delete buf;
}

void logging::logger::critical(const int &line, const std::string &msg)
{
    char *buf = new char[ALIGN_64(msg.size() + delta)];
    sprintf(buf, "[process: %d][module: %s][line:%d] %s ", this->pid, this->module_name, line, msg.c_str());
    spdlog::critical(buf);
    delete buf;
}
