#ifdef _WIN32
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "bot/BotApp.h"
#include "db/Database.h"
#include "services/ReminderService.h"
#include "logging/Logger.h"
using namespace std;
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    const string token = "";
    const string dbPath = "data/PopWordBot.db";
    try {
        Database db(dbPath);
        db.init();
        thread reminderThread([&db, token]() {
            ReminderService reminderService(db, token);
            while (true) {
                this_thread::sleep_for(chrono::hours(6));
                Logger::info("Запуск фоновой рассылки напоминаний...");
                reminderService.sendReminders();
            }
            });
        reminderThread.detach();
        cout << "PopWordBot is running..." << endl;
        BotApp app(token);
        app.run();
    }
    catch (const exception& e) {
        Logger::error("Критическая ошибка в main: " + string(e.what()));
        return 1;
    }
    return 0;
}