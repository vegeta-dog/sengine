//
// Created by longjin on 2022/3/6.
//

#ifndef SENGINE_LOGGER_H
#define SENGINE_LOGGER_H

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"  // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h" // support for user defined types

namespace logging{
    class logger
    {
    public:
        /**
         * 实例化logger
         * @param module 当前实例的模块名
         */
        logger(const std::string &module);
        ~logger()= default;

        void debug(const int &line, const std::string& msg);
        void info(const int &line, const std::string& msg);
        void warn(const int &line, const std::string& msg);
        void error(const int &line, const std::string& msg);
        void critical(const int &line, const std::string& msg);

    private:

        char module_name[4096];
        int pid, tid;
    };
}


#endif //SENGINE_LOGGER_H
