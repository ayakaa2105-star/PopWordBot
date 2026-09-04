#include "ReminderService.h"
#include "logging/Logger.h"
#include <curl/curl.h>
#include <iostream>
#include <vector>
#include <random>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
ReminderService::ReminderService(Database& db, std::string token)
    : db_(db), token_(std::move(token)) {
}
void ReminderService::sendReminders() {
    auto userIds = db_.getUsersWithReminders();
    if (userIds.empty()) {
        Logger::info("Нет пользователей для отправки напоминаний.");
        return;
    }
    std::vector<std::string> reminders = {
        "Pop! Ready for a mini quest? Can you remember this word?",
        "А вы точно помните это слово? Давайте проверим!",
        "⁠Новый квест уже ждёт вас! Смотрите вспомнить слово?",
        "⁠Время проверить память! Вспомните это слово?",
        "⁠A new quest just popped up! Can you remember the word?"
    };
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<size_t> dis(0, reminders.size() - 1);
    std::string random_reminder = reminders[dis(generator)];
    CURL* curl = curl_easy_init();
    if (!curl) {
        Logger::error("Не удалось инициализировать libcurl в ReminderService.");
        return;
    }
    std::string url = "https://api.telegram.org/bot" + token_ + "/sendMessage";
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    for (int64_t userId : userIds) {
        json request = {
            {"chat_id", userId},
            {"text", random_reminder},
            {"reply_markup", {
                {"inline_keyboard", {{
                    {
                        {"text", "🧠 Quiz"},
                        {"callback_data", "open_quiz"}
                    }
                }}}
            }}
        };
        std::string postData = request.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        CURLcode result = curl_easy_perform(curl);
        if (result != CURLE_OK) {
            Logger::error("Ошибка отправки напоминания пользователю: " + std::to_string(userId));
        }
        else {
            Logger::info("Напоминание успешно отправлено: " + std::to_string(userId));
        }
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}