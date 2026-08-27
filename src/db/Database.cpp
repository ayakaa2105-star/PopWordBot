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
	if (db_ != nullptr) {
		sqlite3_close(db_);
	}
}
void Database::exec(const string& sql) {
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
		"user_id INTEGER NOT NULL, "
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
	exec("INSERT OR IGNORE INTO users (user_id) VALUES (" + to_string(userId) + ")");
}
void Database::addWord(const Word& w) {
    string sql = "INSERT INTO words (user_id, word, translation, explanation, example) VALUES"
    "(?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, w.userId);
    sqlite3_bind_text(stmt, 2, w.word.c_str(), -1, SQLITE_TRANSIENT);    
    sqlite3_bind_text(stmt, 3, w.translation.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, w.explanation.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, w.example.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
