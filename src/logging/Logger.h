#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
using namespace std;
class Logger {
public:
    static void info(const string& msg) { log("INFO ", msg); }
    static void warn(const string& msg) { log("WARN ", msg); }
    static void error(const string& msg) { log("ERROR", msg); }

private:
    static void log(const char* level, const string& msg) {
        auto now = chrono::system_clock::now();
        time_t t = chrono::system_clock::to_time_t(now);
        cout << "[" << level << "] " << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S") << " - " << msg << endl;
    }
};