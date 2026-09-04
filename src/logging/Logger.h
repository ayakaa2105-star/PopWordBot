#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <ctime>
class Logger {
public:
    static void info(const std::string& msg) { log("INFO ", msg); }
    static void warn(const std::string& msg) { log("WARN ", msg); }
    static void error(const std::string& msg) { log("ERROR", msg); }

private:
    static void log(const char* level, const std::string& msg) {
        auto now = std::chrono::system_clock::now();
        time_t t = std::chrono::system_clock::to_time_t(now);
        std::cout << "[" << level << "] " << std::put_time(localtime(&t), "%Y-%m-%d %H:%M:%S") << " - " << msg << std::endl;
    }
};