#ifdef _WIN32
#include <windows.h>
#endif
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "bot/BotApp.h"
#include "db/Database.h"
#include "services/ReminderService.h"
#include "logging/Logger.h"
using namespace std;
namespace {
    void setEnvVar(const string& key, const string& value) {
#ifdef _WIN32
        int result = _putenv_s(key.c_str(), value.c_str());
        const char* check = getenv(key.c_str());
        Logger::info(
            "AFTER setEnvVar [" + key + "] length: " +
            to_string(check ? strlen(check) : 0)
        );
        Logger::info(
            "setEnvVar [" + key + "]: " +
            string(result == 0 ? "OK" : "ERROR")
        );
#else
        int result = setenv(key.c_str(), value.c_str(), 1);
        Logger::info(
            "setEnvVar [" + key + "]: " +
            string(result == 0 ? "OK" : "ERROR")
        );
#endif
    }
    void loadDotEnv(const string& path) {
        ifstream file(path);
        if (!file.is_open()) {
            return;
        }
        string line;
        while (getline(file, line)) {
            size_t start = line.find_first_not_of(" \t\r\n");
            if (start == string::npos || line[start] == '#') {
                continue;
            }
            size_t eq = line.find('=');
            if (eq == string::npos) {
                continue;
            }
            string key = line.substr(start, eq - start);
            size_t keyEnd = key.find_last_not_of(" \t");
            key = (keyEnd == string::npos) ? "" : key.substr(0, keyEnd + 1);
            string value = line.substr(eq + 1);
            size_t valStart = value.find_first_not_of(" \t");
            size_t valEnd = value.find_last_not_of(" \t\r\n");
            value = (valStart == string::npos) ? "" : value.substr(valStart, valEnd - valStart + 1);
            if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                (value.front() == '\'' && value.back() == '\''))) {
                value = value.substr(1, value.size() - 2);
            }
            if (key.empty()) {
                continue;
            }
            Logger::info("ENV key loaded: [" + key + "]");
            Logger::info("ENV value length: " + to_string(value.size()));
            setEnvVar(key, value);
        }
    }
    string requireEnv(const char* name) {
        const char* value = getenv(name);
        if (!value || string(value).empty()) {
            throw runtime_error(string("Переменная окружения ") + name + " не задана.");
        }
        return string(value);
    }
}
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    const string dbPath = "data/PopWordBot.db";
    loadDotEnv(".env");
    const char* folderIdFromEnv = getenv("YANDEX_FOLDER_ID");
    if (folderIdFromEnv == nullptr || strlen(folderIdFromEnv) == 0) {
        Logger::error("YANDEX_FOLDER_ID пропал после загрузки .env");
    }
    setEnvVar("YANDEX_FOLDER_ID", "b1gpjavb5l906u2agc6f");
    const char* folderId = getenv("YANDEX_FOLDER_ID");
    Logger::info(
        "AFTER loadDotEnv YANDEX_FOLDER_ID length: " +
        to_string(folderId ? strlen(folderId) : 0)
    );
    Logger::info(
        "YANDEX_FOLDER_ID length: " +
        to_string(folderId ? strlen(folderId) : 0)
    );
    Logger::info("TELEGRAM_BOT_TOKEN: " +
        string(getenv("TELEGRAM_BOT_TOKEN") ? "OK" : "NOT FOUND"));

    Logger::info("YANDEX_API_KEY: " +
        string(getenv("YANDEX_API_KEY") ? "OK" : "NOT FOUND"));

    Logger::info("YANDEX_FOLDER_ID: " +
        string(getenv("YANDEX_FOLDER_ID") ? "OK" : "NOT FOUND"));
    try {
        string token = requireEnv("TELEGRAM_BOT_TOKEN"); 
        Database db(dbPath);
        db.init();
        thread reminderThread([&db, token]() {
            ReminderService reminderService(db, token);
            while (true) {
                this_thread::sleep_for(chrono::hours(2));
                Logger::info("Запуск фоновой рассылки напоминаний...");
                reminderService.sendReminders();
            }
            });
        reminderThread.detach();
        cout << "PopWordBot is running..." << endl;
        BotApp app(token, db);
        app.run();
    }
    catch (const exception& e) {
        Logger::error("Критическая ошибка в main: " + string(e.what()));
        return 1;
    }
    return 0;
}