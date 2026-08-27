#include "Dictionary.h"
#include "../logging/Logger.h"
#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;
//ЦЕЛЬ: ПОЛУЧИТЬ ИНФОРМАЦИЮ О СЛОВЕ ИЗ ВНЕШНЕГО API И ПРАВИЛЬНО ВЕРНУТЬ ЕЕ ПРОГРАММЕ. LIBCURL - ЭТО ИНСТРУМЕНТ С++ КОТОРЫЙ
// УМЕЕТ ДЕЛАТЬ HTTP ЗАПРОСЫ
// JSON - ПОЛУЧАЕТ ОТВЕТ, А NLOHMANJSON - ПЕРЕВОДИТ НА ЯЗЫК С++
static size_t writeCallback(char* dataFromServer, size_t size, size_t numofelements, void* userdata) {
    auto* out = static_cast<string*>(userdata);
    out->append(dataFromServer, size * numofelements);
    return size * numofelements;
}
string getTranslation(const string& englishWord) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        cout << "Init failed! \n";
        return "ERROR!";
    }
    string answer;
    json request;
    request["folderId"] = "b1gpjavb5l906u2agc6f";
    request["texts"] = { englishWord };
    request["targetLanguageCode"] = "ru";
    string requestText = request.dump();
    string url = "https://translate.api.cloud.yandex.net/translate/v2/translate";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &answer);
    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers,"Authorization: ");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestText.c_str());
    CURLcode result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        Logger::warn("Ошибка запроса к Yandex Translate");
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return "ERROR!";
    }
    json data = json::parse(answer);
    string translation =
        data["translations"][0]["text"];
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return translation;
}
optional<WordInfo> DictionaryClient::lookup(const string& englishWord) {
    CURL* curl;
    curl = curl_easy_init();
    if (!curl) {
        cout << "init ffailed!\n";
        return nullopt;
    }
    string answer;
    string url = "https://api.dictionaryapi.dev/api/v2/entries/en/" + englishWord; //собрать ссылку на сайт-словарь чтоб для каждого слова был уник адрес запроса
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &answer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    CURLcode result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        cout << "ERROR!";
    }
    json data = json::parse(answer);
    string word = data[0]["word"];
    string definition = data[0]["meanings"][0]["definitions"][0]["definition"];
    string example = data[0]["meanings"][0]["definitions"][0]["example"];
    cout << word << " - " << definition << endl;
    cout << "Example: " << example << endl;
    if (result == CURLE_OK) {
        long http_code{ 0 }; //we got http-cpde answer from the server
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code == 200) {
            cout << "Request completed succesfully!" << endl;
        }
        else {
            Logger::warn("...");
            return nullopt;
        }
    }
    string translation = getTranslation(englishWord);
    WordInfo res;
    res.translation = translation;
    res.explanation = definition;
    res.example = example;
    curl_easy_cleanup(curl);
    return res;
}
