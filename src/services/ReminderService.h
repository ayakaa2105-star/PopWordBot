#pragma once
#include <string>
#include "db/Database.h"
class ReminderService {
public:
    ReminderService(Database& db, std::string token);
    void sendReminders();
private:
    Database& db_;
    std::string token_;
};