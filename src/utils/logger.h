#pragma once
#include <string>

#ifdef ENABLE_LOGGING
    #define LOG_INFO(msg) Logger::info(msg)
    #define LOG_ERROR(msg) Logger::error(msg)
#else
    #define LOG_INFO(msg)
    #define LOG_ERROR(msg)
#endif

class Logger {
public:
    static void info(const std::string& msg);
    static void error(const std::string& msg);
};