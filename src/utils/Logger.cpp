#include "utils/Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

static std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void Logger::info(const std::string& msg) {
    fmt::print("[{}] [INFO]  {}\n", getTimestamp(), msg);
}

void Logger::error(const std::string& msg) {
    fmt::print(stderr, "[{}] [ERROR] {}\n", getTimestamp(), msg);
}

void Logger::warn(const std::string& msg) {
    fmt::print("[{}] [WARN]  {}\n", getTimestamp(), msg);
}

void Logger::debug(const std::string& msg) {
    fmt::print("[{}] [DEBUG] {}\n", getTimestamp(), msg);
}