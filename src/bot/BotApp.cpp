#include "BotApp.h"
#include "Keyboards.h"
#include "../logging/Logger.h"
using namespace std;
BotApp::BotApp(const string& token) : bot_(token), db_("data/popword.db")//, quiz_(db_) 
{
	db_init();
	registratHandlers();
}
void BotApp::registratHandlers() {
	bot_.getEvents().onCommand("start", [this](TgBot::Message::Ptr message) {
		onStartCommand(message);
		});
	bot_.getEvents().onAnyMessage([this](TgBot::Message::Ptr message) {
		if (!message->text.empty() && message->text[0] == '/') {
			return;
		}
		onText(message);
		});
	bot_.getEvents().onCallbackQuery([this](TgBot::CallbackQuery::Ptr query) {
		onCallbackQuery(query);
		});
}
void BotApp::run() {
	try {
		TgBot::TgLongPoll longPoll(bot_);
		while (true) {
			longPoll.start();
		}
	}
	catch (const exception& error) {
		Logger::error("Ошибка в работе бота: " + string(error.what()));
	}
}
void BotApp::onStartCommand(TgBot::Message::Ptr message) {
	int64_t userId = message->from->id;
	int64_t chatId = message->chat->id;
	db_.ensureUser(userId);
	userStates_[chatId] = UserState::IDLE;
	bot_.getApi().sendMessage(
		chatId,
		"👋 Welcome to PopWord!\n"
		"Здесь учить английские слова проще чем кажется.🧠💫\n\n"
		"Сейчас это бета - версия Telegram - бота, которую мы постепенно развиваем и улучшаем.\n"
		"С помощью PopWord вы можете добавлять новую лексику, практиковать её с помощью коротких игр - квизов и возвращаться к словам, чтобы они действительно запоминались.\n",
		nullptr,
		nullptr,
		Keyboards::mainMenu()
	);
}
void BotApp::onText(TgBot::Message::Ptr message) {
	int64_t chatId = message->chat->id;
	UserState state = UserState::IDLE;
	if (userStates_.count(chatId)) {
		state = userStates_[chatId];
	}
	if (state == UserState::AWAITING_WORD) {
		handleAddedWord(message);
	}
	else if (state == UserState::IDLE) {
		handleMainMenuButton(message);
	}
	else if (state == UserState::IN_QUIZ) {
		Logger::warn(
			"Пользователь отправил текст во время квиза: " + to_string(message->from->id)
		);
		bot_.getApi().sendMessage(chatId, "Пожалуйста, используйте кнопки для ответа.");
	}
}
void BotApp::handleMainMenuButton(TgBot::Message::Ptr message) {
	int64_t chatId = message->chat->id;
	int64_t userId = message->from->id;
	if (message->text == "Добавить слово") {
		userStates_[chatId] = UserState::AWAITING_WORD;
		bot_.getApi().sendMessage(chatId, "Введите слово:");
	}
	else if (message->text == "Библиотека") {
		Logger::info("Пользователь открыл библиотеку: " + to_string(userId));
		bot_.getApi().sendMessage(chatId, "Библиотека пока находится в разработке");
	}
else if (message->text == "Quiz") {
	Logger::info("Пользователь пытался запустить квиз: " + to_string(userId));
	bot_.getApi().sendMessage(chatId, "Quiz пока находится в разработке");
}
else {
	Logger::warn(
		"Непонятный ввод от пользователя" + to_string(userId)
	);
	bot_.getApi().sendMessage(chatId, "Пожалуйста, используйте кнопки меню.");
	}
}
void BotApp::handleAddedWord(TgBot::Message::Ptr message) {
	int64_t chatId = message->chat->id;
	int64_t userId = message->from->id;
	if (!isValidWordInput(message->text)) {
		Logger::warn("Некорректный ввод слова от пользователя: " + to_string(userId));
		bot_.getApi(sendMessage(chatId, "Некорректное слово.\n Введите английское слово латинскими буквами!");
		return;
	}
	bool isFirstWord = != firstAddedWord(chatId, "Слово " + message->text + "записано!");
	if (isFirstWord) {
		bot_.getApi().sendMessage(chatId, "Для эффективного запоминания слов включите уведомления в настройках Telegram для этого бота!");
		firstWordAdded_.insert(userId);
	}
	UserStates_[chatId] = UserState::IDLE;
	bot_.getApi().sendMessage(chatId, "Menu:", nullptr, nullptr, Keyboards::mainMenu());
}
bool BotApp::isValidWordInput(const string& text) const {
	if (text.empty()) {
		return false;
	}
	if (text.length() > 30) {
		return false;
	}
	for (char symbol : text) {
		if (!(symbol >= 'a' && symbol <= 'z') ||
			(symbol >= 'A' && symbol <= 'Z')) {
			return false;
		}
	}
	return true;
}
void BotApp::startQuiz(int64_t chatId, int64_t userId) {
	Logger::info("Quiz еще не реализован для пользователя: " + to_string(userId));
	bot_.getApi().sendMessage(chatId, "Quiz пока находится в разработке.");
}
void BotApp::onCallbackQuery(TgBot::CallbackQuery::Ptr query) {
	bot_.getApi().answerCallbackQuery(query->id);
	Logger::info("Нажатие inline кнопки.");
}