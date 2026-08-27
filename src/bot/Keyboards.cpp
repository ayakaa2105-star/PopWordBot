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

