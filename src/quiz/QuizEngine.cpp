#include "QuizEngine.h"
#include "../db/Database.h"
#include <random>
#include <algorithm>
#include <vector>
using namespace std;
optional<QuizQuestion> QuizEngine::generateQuestion(int64_t userId) {
    vector<Word> words = db_.getWordsByUser(userId);
    if (words.size() < 4) {
        return nullopt;
    }
    int correctWordId = rand() % words.size();
    Word correctWord = words[correctWordId];
    words.erase(words.begin() + correctWordId);
    vector <string> options; //неправильные ответы + правильные
    for (int i{ 0 }; i < 3; i++) {
        int randomWordId = rand() % words.size();
        options.push_back(words[randomWordId].translation);
        words.erase(words.begin() + randomWordId);
    }
    options.push_back(correctWord.translation);
    for (int i{ 0 }; i < 10; i++) { //перемешаем
        int a = rand() % 4;
        int b = rand() % 4;
        string temp = options[a];
        options[a] = options[b];
        options[b] = temp;
    }
    auto it = find(options.begin(), options.end(), correctWord.translation);
    int correctOptionIndex = distance(options.begin(), it);
    QuizQuestion q;
    q.correctWord = correctWord;
    q.options = options;
    q.correctOptionIndex = correctOptionIndex;
    return q;
}