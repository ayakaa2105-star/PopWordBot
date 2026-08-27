#pragma once
#include <string>
#include <optional>
using namespace std;
struct WordInfo {
    string translation;
    string explanation;
    string example;
};
class DictionaryClient {
public:
    optional<WordInfo> lookup(const string& englishWord);
private:
    string getTranslation(const string& englishWord);
};