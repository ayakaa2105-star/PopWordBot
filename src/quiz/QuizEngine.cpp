#include "QuizEngine.h"
#include "../db/Database.h"
#include <random>
#include <algorithm>
#include <vector>
using namespace std;
namespace {
    mt19937& rng() {
        static mt19937 generator(random_device{}());
        return generator;
    }
}
optional<QuizQuestion> QuizEngine::generateQuestion(int64_t userId) {
    vector<Word> words = db_.getWordsByUser(userId);
    if (words.size() < 4) {
        return nullopt;
    }
    uniform_int_distribution<size_t> pickAny(0, words.size() - 1);
    size_t correctWordId = pickAny(rng());
    Word correctWord = words[correctWordId];
    words.erase(words.begin() + correctWordId);
    vector<string> options;
    for (int i = 0; i < 3; i++) {
        uniform_int_distribution<size_t> pickRemaining(0, words.size() - 1);
        size_t randomWordId = pickRemaining(rng());
        options.push_back(words[randomWordId].translation);
        words.erase(words.begin() + randomWordId);
    }
    options.push_back(correctWord.translation);
    shuffle(options.begin(), options.end(), rng());
    auto it = find(options.begin(), options.end(), correctWord.translation);
    int correctOptionIndex = static_cast<int>(distance(options.begin(), it));
    QuizQuestion q;
    q.correctWord = correctWord;
    q.options = options;
    q.correctOptionIndex = correctOptionIndex;
    return q;
}