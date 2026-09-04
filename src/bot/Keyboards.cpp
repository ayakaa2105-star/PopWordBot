#include "Keyboards.h"
using namespace std;
TgBot::ReplyKeyboardMarkup::Ptr Keyboards::mainMenu() {
	auto keyboard = make_shared<TgBot::ReplyKeyboardMarkup>();
	keyboard->resizeKeyboard = true;
	auto addWord = make_shared<TgBot::KeyboardButton>();
	addWord->text = "Добавить слово";
	auto library = make_shared<TgBot::KeyboardButton>();
	library->text = "Библиотека";
	auto quiz = make_shared<TgBot::KeyboardButton>();
	quiz->text = "Quiz";
	vector<TgBot::KeyboardButton::Ptr> row1;
	row1.push_back(addWord);
	keyboard->keyboard.push_back(row1);
	vector<TgBot::KeyboardButton::Ptr> row2;
	row2.push_back(library);
	row2.push_back(quiz);
	keyboard->keyboard.push_back(row2);
	return keyboard;
}
TgBot::InlineKeyboardMarkup::Ptr Keyboards::quizOptions(const vector<string>& options) {
	auto keyboard = make_shared<TgBot::InlineKeyboardMarkup>();
	for (int i{ 0 }; i < options.size(); i++) {
		auto button = make_shared<TgBot::InlineKeyboardButton>();
		button->text = options[i];
		button->callbackData = "quiz_" + to_string(i);
		vector <TgBot::InlineKeyboardButton::Ptr> row;
		row.push_back(button);
		keyboard->inlineKeyboard.push_back(row);
	}
	return keyboard;
}

