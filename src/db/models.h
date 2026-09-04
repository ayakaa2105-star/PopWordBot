#pragma once
#include <string>
#include <cstdint>
struct Word {
    int64_t id = 0;
    int64_t userId = 0;
    std::string word;
    std::string translation;
    std::string explanation;
    std::string example;
};