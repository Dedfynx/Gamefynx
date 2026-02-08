#pragma once
#include <fmt/core.h>
#include <fmt/format.h>
#include <string>

#ifdef ENABLE_LOGGING
    #define LOG_INFO(...)  Logger::info(fmt::format(__VA_ARGS__))
    #define LOG_ERROR(...) Logger::error(fmt::format(__VA_ARGS__))
    #define LOG_WARN(...)  Logger::warn(fmt::format(__VA_ARGS__))
    #define LOG_DEBUG(...) Logger::debug(fmt::format(__VA_ARGS__))
#else
    #define LOG_INFO(...)
    #define LOG_ERROR(...)
    #define LOG_WARN(...)
    #define LOG_DEBUG(...)
#endif

class Logger {
public:
    static void info(const std::string& msg);
    static void error(const std::string& msg);
    static void warn(const std::string& msg);
    static void debug(const std::string& msg);
};