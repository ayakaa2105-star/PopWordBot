#pragma once
#include <vector>
#include <string>
#include <optional>
#include "../db/models.h"
struct QuizQuestion {
    Word correctWord;
    std::vector<std::string> options;
    int correctOptionIndex = 0;
};
class Database;
class QuizEngine {
public:
    explicit QuizEngine(Database& db) : db_(db) {}
    std::optional<QuizQuestion> generateQuestion(int64_t userId);
private:
    Database& db_;
};