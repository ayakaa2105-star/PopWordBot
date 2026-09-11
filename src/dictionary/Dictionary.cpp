#include "Dictionary.h"
#include "../logging/Logger.h"
#include <curl/curl.h>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;
namespace {
    size_t writeCallback(char* dataFromServer, size_t size, size_t numofelements, void* userdata) {
        auto* out = static_cast<string*>(userdata);
        out->append(dataFromServer, size * numofelements);
        return size * numofelements;
    }
    string getEnvOrEmpty(const char* name) {
        const char* value = std::getenv(name);
        return value ? string(value) : string();
    }
    void applyCommonOptions(CURL* curl) {
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        if (!getEnvOrEmpty("CURL_VERBOSE").empty()) {
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        }
    }
    string stripCodeFence(string text) {
        size_t start = text.find("```");
        if (start == string::npos) {
            return text;
        }
        size_t contentStart = text.find('\n', start);
        if (contentStart == string::npos) {
            return text;
        }
        size_t end = text.rfind("```");
        if (end == string::npos || end <= contentStart) {
            return text;
        }
        return text.substr(contentStart + 1, end - contentStart - 1);
    }
    optional<string> askYandexGpt(const string& folderId, const string& apiKey, const string& englishWord) {
        json request;
        request["modelUri"] = "gpt://" + folderId + "/yandexgpt-lite";
        request["completionOptions"] = {
            {"stream", false},
            {"temperature", 0.2},
            {"maxTokens", "300"}
        };
        request["messages"] = json::array({
           {
               {"role", "system"},
{"text",
    "Ты помощник для изучения английского языка. Тебе дают одно "
    "английское слово. Создай для него полезную и понятную словарную "
    "карточку для человека, который изучает английский язык. "

    "Ответь СТРОГО в формате JSON без каких-либо пояснений вне JSON "
    "и без markdown-обёртки: "

    "{\"translation\": \"основные распространённые переводы слова на русский\", "
    "\"explanation\": \"подробное и понятное объяснение значения и оттенков слова на русском\", "
    "\"example\": \"одно или два естественных предложения на английском, "
    "которые помогают понять значение и употребление слова\"}. "

    "Правила для translation: "
    "укажи все основные распространённые значения и переводы слова, "
    "если их несколько. Не ограничивайся одним переводом. "
    "Если у слова есть несколько естественных вариантов перевода, "
    "укажи их через запятую. Не добавляй редкие, устаревшие или "
    "неподходящие по смыслу переводы только ради увеличения количества. "
    "Если у слова только один основной перевод, укажи только его. "

    "Правила для explanation: "
    "дай короткое и понятное описание значения слова в 2-3 предложениях. "
    "Объясни, что именно означает слово и в каких ситуациях его используют. "
    "Если слово используется как в прямом, так и в переносном смысле, "
    "кратко объясни оба значения. Например, если слово может описывать "
    "физический объект и эмоциональное состояние человека, укажи оба случая. "
    "Если слово имеет важный эмоциональный оттенок, кратко объясни его. "
    "Не повторяй просто перевод и не используй сложный академический язык. "

    "Правила для example: "
    "дай одно или два естественных предложения на английском. "
    "Предложения не должны быть слишком короткими или искусственными. "
    "Они должны показывать реальный контекст употребления слова, "
    "чтобы изучающий мог понять его значение непосредственно из ситуации. "
    "Если у слова есть важный эмоциональный или смысловой оттенок, "
    "пример должен показывать именно этот оттенок. "

    "Если это не настоящее английское слово или его значение невозможно "
    "определить, ответь ровно "
    "{\"error\": \"not_found\"}."
                }
            },
            {
                {"role", "user"},
                {"text", englishWord}
            }
            });
        string requestText = request.dump();
        const int maxAttempts = 3;
        for (int attempt = 1; attempt <= maxAttempts; attempt++) {
            CURL* curl = curl_easy_init();
            if (!curl) {
                Logger::error("Не удалось инициализировать libcurl для YandexGPT.");
                return nullopt;
            }
            string answer;
            string url = "https://llm.api.cloud.yandex.net/foundationModels/v1/completion";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &answer);
            applyCommonOptions(curl);
            curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("Authorization: Api-Key " + apiKey).c_str());
            headers = curl_slist_append(headers, ("x-folder-id: " + folderId).c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestText.c_str());
            CURLcode result = curl_easy_perform(curl);
            long httpCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            if (result != CURLE_OK) {
                Logger::warn("Ошибка запроса к YandexGPT (попытка " + to_string(attempt) + "/" +
                    to_string(maxAttempts) + "): " + curl_easy_strerror(result));
                if (attempt < maxAttempts) {
                    this_thread::sleep_for(chrono::milliseconds(800));
                    continue;
                }
                return nullopt;
            }
            if (httpCode != 200) {
                Logger::warn("YandexGPT вернул код " + to_string(httpCode) + ": " + answer);
                if (attempt < maxAttempts && (httpCode == 429 || httpCode >= 500)) {
                    this_thread::sleep_for(chrono::milliseconds(800));
                    continue;
                }
                return nullopt;
            }
            return answer;
        }
        return nullopt;
    }
}
optional<WordInfo> DictionaryClient::lookup(const string& englishWord) {
    string folderId = getEnvOrEmpty("YANDEX_FOLDER_ID");
    string apiKey = getEnvOrEmpty("YANDEX_API_KEY");
    if (folderId.empty() || apiKey.empty()) {
        Logger::error("YANDEX_FOLDER_ID / YANDEX_API_KEY не заданы в переменных окружения.");
        return nullopt;
    }
    optional<string> rawResponse = askYandexGpt(folderId, apiKey, englishWord);
    if (!rawResponse) {
        return nullopt;
    }
    string modelText;
    try {
        json data = json::parse(*rawResponse);
        modelText = data.at("result").at("alternatives").at(0).at("message").at("text").get<string>();
    }
    catch (const json::exception& e) {
        Logger::warn(string("Не удалось разобрать ответ YandexGPT: ") + e.what());
        return nullopt;
    }
    try {
        json parsed = json::parse(stripCodeFence(modelText));
        if (parsed.contains("error")) {
            Logger::warn("YandexGPT не распознал слово: " + englishWord);
            return nullopt;
        }
        WordInfo res;
        res.translation = parsed.value("translation", "");
        res.explanation = parsed.value("explanation", "");
        res.example = parsed.value("example", "");
        if (res.translation.empty()) {
            return nullopt;
        }
        return res;
    }
    catch (const json::exception& e) {
        Logger::warn(string("YandexGPT вернул не-JSON текст для '") + englishWord + "': " + e.what());
        return nullopt;
    }
}
