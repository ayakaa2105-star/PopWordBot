#pragma once
#include <string>
#include <cstdint>
using namespace std;
struct Word {
    int64_t id = 0;
    int64_t userId = 0;
    string word;
    string translation;
    string explanation;
    string example;
};