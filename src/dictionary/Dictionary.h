#pragma once
#include <string>
#include <optional>
struct WordInfo {
    std::string translation;
    std::string explanation;
    std::string example;
};
class DictionaryClient {
public:
    std::optional<WordInfo> lookup(const std::string& englishWord);
private:
    std::string getTranslation(const std::string& englishWord);
};