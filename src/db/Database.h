#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include "models.h"
class Database {
public:
    explicit Database(const std::string& path);
    ~Database();
    void init();
    void addWord(const Word& w);
    void ensureUser(int64_t userId);
    std::vector<Word> getWordsByUser(int64_t userId);
    std::vector<Word> getRandomWords(int64_t userId, int n);
    void setReminders(int64_t userId, bool enabled);
    std::vector<int64_t> getUsersWithReminders();
private:
    sqlite3* db_ = nullptr;
    void exec(const std::string& sql);
};