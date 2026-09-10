#include "BotApp.h"
#include "Keyboards.h"
#include "../logging/Logger.h"
#include "../db/Database.h"
#include <thread>
#include <chrono>
using namespace std;
BotApp::BotApp(const string& token, Database& db) : bot_(token), db_(db), quiz_(db_) {
	registratHandlers();
}
void BotApp::registratHandlers() {
	bot_.getEvents().onCommand("start", [this](TgBot::Message::Ptr message) {
		onStartCommand(message);
		});
	bot_.getEvents().onAnyMessage([this](TgBot::Message::Ptr message) {
		if (!message->text.has_value() || message->text->empty() || message->text->front() == '/') {
			return;
		}
		onText(message);
		});
	bot_.getEvents().onCallbackQuery([this](TgBot::CallbackQuery::Ptr query) {
		onCallbackQuery(query);
		});
}
void BotApp::run() {
	while (true) {
		try {
			TgBot::TgLongPoll longPoll(bot_);
			while (true) {
				longPoll.start();
			}
		}
		catch (const exception& error) {
			Logger::error("Ошибка в работе бота: " + string(error.what()) + ". Повторная попытка через 5 секунд.");
			this_thread::sleep_for(chrono::seconds(5));
		}
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
		"С помощью PopWord вы можете добавлять новую лексику, практиковать её с помощью коротких игр - квизов и возвращаться к словам, чтобы они действительно запоминались.\n"
		"Готовы начать? 🚀\n"
		"Добавьте свое первое слово!",
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
		Logger::info("Пользователь запустил квиз: " + to_string(userId));
		startQuiz(chatId, userId);
	}
	else {
		Logger::warn("Непонятный ввод от пользователя" + to_string(userId));
		bot_.getApi().sendMessage(chatId, "Пожалуйста, используйте кнопки меню.");
	}
}
void BotApp::handleAddedWord(TgBot::Message::Ptr message) {
	int64_t chatId = message->chat->id;
	int64_t userId = message->from->id;
	if (!message->text.has_value()) {
		return;
	}
	const std::string& text = message->text.value();
	if (!isValidWordInput(text)) {
		Logger::warn("Некорректный ввод слова от пользователя: " + to_string(userId));
		bot_.getApi().sendMessage(chatId, "Некорректное слово.\nВведите английское слово латинскими буквами!");
		return;
	}
	bot_.getApi().sendMessage(chatId, "🔎 Ищу перевод...");
	auto info = dictionary_.lookup(text);
	if (!info) {
		Logger::warn("Не удалось получить перевод слова: " + text);
		bot_.getApi().sendMessage(
			chatId,
			"Не удалось найти это слово в словаре 😔\n"
			"Проверьте написание или попробуйте другое слово."
		);
		return;
	}
	Word word;
	word.userId = userId;
	word.word = text;
	word.translation = info->translation;
	word.explanation = info->explanation;
	word.example = info->example;
	db_.addWord(word);
	string reply = "Слово \"" + text + "\" записано!";
	if (!word.translation.empty()) {
		reply += "\n\n" + text + " — " + word.translation;
	}
	else {
		reply += "\n\n⚠️ Перевод временно недоступен, но слово сохранено — можно будет перевести его позже.";
	}
	if (!word.explanation.empty()) {
		reply += "\n\nОписание: " + word.explanation;
	}
	if (!word.example.empty()) {
		reply += "\n\nПример: " + word.example;
	}
	bot_.getApi().sendMessage(chatId, reply);
	bool isFirstWord = (db_.getWordsByUser(userId).size() == 1);
	if (isFirstWord && firstWordAdded_.find(userId) == firstWordAdded_.end()) {
		bot_.getApi().sendMessage(chatId, "Для эффективного запоминания слов включите уведомления в настройках Telegram для этого бота.");
		firstWordAdded_.insert(userId);
	}
	userStates_[chatId] = UserState::IDLE;
	bot_.getApi().sendMessage(
		chatId,
		"Menu:",
		nullptr,
		nullptr,
		Keyboards::mainMenu()
	);
}
bool BotApp::isValidWordInput(const string& text) const {
	if (text.empty()) {
		return false;
	}
	if (text.length() > 30) {
		return false;
	}
	for (char symbol : text) {
		if (!((symbol >= 'a' && symbol <= 'z') ||
			(symbol >= 'A' && symbol <= 'Z'))) {
			return false;
		}
	}
	return true;
}
void BotApp::startQuiz(int64_t chatId, int64_t userId) {
	auto question = quiz_.generateQuestion(userId);
	if (!question) {
		bot_.getApi().sendMessage(chatId, "Для Quiz нужно минимум 4 слова в библиотеке.");
		return;
	}
	userStates_[chatId] = UserState::IN_QUIZ;
	activeQuiz_[chatId] = *question;
	bot_.getApi().sendMessage(chatId, "🧠 What does " + question->correctWord.word + " mean?", nullptr, nullptr, Keyboards::quizOptions(question->options));
	Logger::info("Quiz запущен для пользователя: " + to_string(userId));
}
namespace {
	int64_t chatIdFromCallbackMessage(const TgBot::MaybeInaccessibleMessage::Ptr& message) {
		if (!message) {
			return 0;
		}
		return std::visit([](auto&& m) -> int64_t {
			return (m && m->chat) ? m->chat->id : 0;
			}, message->value);
	}
}
void BotApp::onCallbackQuery(TgBot::CallbackQuery::Ptr query) {
	int64_t chatId = chatIdFromCallbackMessage(query->message);
	int64_t userId = query->from->id;
	auto it = activeQuiz_.find(chatId);
	if (it == activeQuiz_.end()) {
		bot_.getApi().answerCallbackQuery(query->id, "Этот квиз уже закрыт.");
		return;
	}
	const QuizQuestion& question = it->second;
	if (!query->data.has_value()) {
		bot_.getApi().answerCallbackQuery(query->id);
		return;
	}
	const string& data = query->data.value();
	const string prefix = "quiz_";
	if (data.rfind(prefix, 0) != 0) {
		bot_.getApi().answerCallbackQuery(query->id);
		return;
	}
	int chosenIndex = -1;
	try {
		chosenIndex = stoi(data.substr(prefix.size()));
	}
	catch (const exception&) {
		bot_.getApi().answerCallbackQuery(query->id);
		return;
	}
	bool correct = (chosenIndex == question.correctOptionIndex);
	bot_.getApi().answerCallbackQuery(query->id, correct ? "✅" : "❌");
	if (correct) {
		bot_.getApi().sendMessage(chatId, "✅ Correct! " + question.correctWord.word + " = " + question.correctWord.translation);
	}
	else {
		bot_.getApi().sendMessage(
			chatId,
			"💡 Almost!\nПравильный ответ: " + question.correctWord.word + " - " + question.correctWord.translation + ". \nTry to remember it for next time!"
		);
	}
	activeQuiz_.erase(it);
	userStates_[chatId] = UserState::IDLE;
	Logger::info("Ответ на квиз от пользователя " + to_string(userId) + ": " + (correct ? "верно" : "неверно"));
	bot_.getApi().sendMessage(chatId, "Menu:", nullptr, nullptr, Keyboards::mainMenu());
}