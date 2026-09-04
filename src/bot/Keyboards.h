#pragma once
#include <tgbot/tgbot.h>
namespace Keyboards {
	TgBot::ReplyKeyboardMarkup::Ptr mainMenu();
	TgBot::InlineKeyboardMarkup::Ptr quizOptions(const std::vector<std::string>& options);
}