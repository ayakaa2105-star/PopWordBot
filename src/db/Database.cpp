#include <iostream>
#include <string>
#include "Database.h"
#include "../logging/Logger.h"
using namespace std;
string getTextColumn(sqlite3_stmt* stmt, int cmnIdx) {
    const unsigned char* rawText = sqlite3_column_text(stmt, cmnIdx);
    const char* text = (const char*)rawText;
    if (text == nullptr) {
        return "";
    }
    return string(text);
}
Database::Database(const string& path) {
    int result = sqlite3_open(path.c_str(), &db_);
    if (result == SQLITE_OK) {
        cout << "Подключение установлено\n";
    }
    else {
        Logger::error("ERROR! Не удалось открыть базу данных: " + path);
    }
}
Database::~Database() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}
void Database::exec(const string& sql) {
    lock_guard<mutex> lock(mutex_);
    char* errMsg = nullptr;
    int rslt = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rslt != SQLITE_OK) {
        Logger::error(string("Ошибка в SQL: ") + errMsg);
        sqlite3_free(errMsg);
    }
}
void Database::init() {
    exec(
        "CREATE TABLE IF NOT EXISTS users("
        "user_id INTEGER NOT NULL UNIQUE, "
        "reminders_enabled BOOLEAN DEFAULT 0"
        ");"
    );
    exec(
        "CREATE TABLE IF NOT EXISTS words("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "user_id INTEGER NOT NULL, "
        "word TEXT NOT NULL, "
        "translation TEXT, "
        "explanation TEXT, "
        "example TEXT, "
        "added_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");"
    );
}
void Database::ensureUser(int64_t userId) {
    lock_guard<mutex> lock(mutex_);
    string sql = "INSERT OR IGNORE INTO users (user_id) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;
    int prepareResult = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (prepareResult != SQLITE_OK || stmt == nullptr) {
        Logger::error("Ошибка подготовки запроса ensureUser: " + string(sqlite3_errmsg(db_)));
        return;
    }
    sqlite3_bind_int64(stmt, 1, userId);
    int stepResult = sqlite3_step(stmt);
    if (stepResult != SQLITE_DONE) {
        Logger::error("Ошибка выполнения ensureUser: " + string(sqlite3_errmsg(db_)));
    }
    sqlite3_finalize(stmt);
}
void Database::addWord(const Word& w) {
    lock_guard<mutex> lock(mutex_);
    string sql = "INSERT INTO words (user_id, word, translation, explanation, example) VALUES"
        "(?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    int prepareResult = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (prepareResult != SQLITE_OK || stmt == nullptr) {
        Logger::error("Ошибка подготовки запроса addWord: " + string(sqlite3_errmsg(db_)));
        return;
    }
    sqlite3_bind_int64(stmt, 1, w.userId);
    sqlite3_bind_text(stmt, 2, w.word.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, w.translation.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, w.explanation.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, w.example.c_str(), -1, SQLITE_TRANSIENT);
    int stepResult = sqlite3_step(stmt);
    if (stepResult != SQLITE_DONE) {
        Logger::error("Ошибка выполнения addWord: " + string(sqlite3_errmsg(db_)));
    }
    sqlite3_finalize(stmt);
}
vector<Word> Database::getWordsByUser(int64_t userId) {
    lock_guard<mutex> lock(mutex_);
    vector<Word> out;
    string sql = "SELECT id, user_id, word, translation, explanation, example FROM words WHERE user_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    int prepareResult = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (prepareResult != SQLITE_OK || stmt == nullptr) {
        Logger::error("Ошибка подготовки запроса getWordsByUser: " + string(sqlite3_errmsg(db_)));
        return out;
    }
    sqlite3_bind_int64(stmt, 1, userId);
    int stepResult = sqlite3_step(stmt);
    while (stepResult == SQLITE_ROW) {
        Word w;
        w.id = sqlite3_column_int64(stmt, 0);
        w.userId = sqlite3_column_int64(stmt, 1);
        w.word = getTextColumn(stmt, 2);
        w.translation = getTextColumn(stmt, 3);
        w.explanation = getTextColumn(stmt, 4);
        w.example = getTextColumn(stmt, 5);
        out.push_back(w);
        stepResult = sqlite3_step(stmt);
    }
    if (stepResult != SQLITE_DONE) {
        Logger::error("Ошибка выполнения getWordsByUser: " + string(sqlite3_errmsg(db_)));
    }
    sqlite3_finalize(stmt);
    return out;
}
vector<Word> Database::getRandomWords(int64_t userId, int n) {
    lock_guard<mutex> lock(mutex_);
    vector<Word> result;
    string sql = "SELECT id, user_id, word, translation, explanation, example FROM words WHERE user_id = ? ORDER BY RANDOM() LIMIT ?;";
    sqlite3_stmt* stmt = nullptr;
    int prepareResult = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (prepareResult != SQLITE_OK || stmt == nullptr) {
        Logger::error("Ошибка подготовки запроса getRandomWords: " + string(sqlite3_errmsg(db_)));
        return result;
    }
    sqlite3_bind_int64(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, n);
    int stepResult = sqlite3_step(stmt);
    while (stepResult == SQLITE_ROW) {
        Word w;
        w.id = sqlite3_column_int64(stmt, 0);
        w.userId = sqlite3_column_int64(stmt, 1);
        w.word = getTextColumn(stmt, 2);
        w.translation = getTextColumn(stmt, 3);
        w.explanation = getTextColumn(stmt, 4);
        w.example = getTextColumn(stmt, 5);
        result.push_back(w);
        stepResult = sqlite3_step(stmt);
    }
    if (stepResult != SQLITE_DONE) {
        Logger::error("Ошибка выполнения getRandomWords: " + string(sqlite3_errmsg(db_)));
    }
    sqlite3_finalize(stmt);
    return result;
}
vector<int64_t> Database::getUsersWithReminders() {
    lock_guard<mutex> lock(mutex_);
    vector<int64_t> result;
    string sql = "SELECT user_id FROM users WHERE reminders_enabled = 1;";
    sqlite3_stmt* stmt = nullptr;
    int prepareResult = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (prepareResult != SQLITE_OK || stmt == nullptr) {
        Logger::error("Ошибка подготовки запроса getUsersWithReminders: " + string(sqlite3_errmsg(db_)));
        return result;
    }
    int stepResult = sqlite3_step(stmt);
    while (stepResult == SQLITE_ROW) {
        int64_t userId = sqlite3_column_int64(stmt, 0);
        result.push_back(userId);
        stepResult = sqlite3_step(stmt);
    }
    if (stepResult != SQLITE_DONE) {
        Logger::error("Ошибка выполнения getUsersWithReminders: " + string(sqlite3_errmsg(db_)));
    }
    sqlite3_finalize(stmt);
    return result;
}
void Database::setReminders(int64_t userId, bool enabled) {
    lock_guard<mutex> lock(mutex_);
    string sql = "UPDATE users SET reminders_enabled = ? WHERE user_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    int prepareResult = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (prepareResult != SQLITE_OK || stmt == nullptr) {
        Logger::error("Ошибка подготовки запроса setReminders: " + string(sqlite3_errmsg(db_)));
        return;
    }
    int v = 0;
    if (enabled) {
        v = 1;
    }
    sqlite3_bind_int(stmt, 1, v);
    sqlite3_bind_int64(stmt, 2, userId);
    int stepResult = sqlite3_step(stmt);
    if (stepResult != SQLITE_DONE) {
        Logger::error("Ошибка выполнения setReminders: " + string(sqlite3_errmsg(db_)));
    }
    sqlite3_finalize(stmt);
}
