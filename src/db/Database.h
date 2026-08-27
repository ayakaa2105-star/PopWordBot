#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include "models.h"
using namespace std;
class Database {
public:
    explicit Database(const string& path);
    ~Database();
    void init();
    void addWord(const Word& w);
    void ensureUser(int64_t userId);
private:
    sqlite3* db_ = nullptr;
    void exec(const string& sql);
};