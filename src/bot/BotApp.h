#pragma once
#include <tgbot/tgbot.h>
#include <unordered_map>
#include <unordered_set>
#include "../db/Database.h"
//#include "../dictionary/Dictionary.h"
//#include "../quiz/QuizEngine.h"
using namespace std;
enum class UserState {
	IDLE, AWAITING_WORD, IN_QUIZ
};
class BotApp {
public:
	explicit BotApp(const string& token);
	void run();
private:
	TgBot::Bot bot_;
	Database db_;
	//Dictionary dictionary_;
	//QuizEngine quiz_;
	unordered_map<int64_t, UserState> userStates_;
	unordered_set<int64_t> turnonnot1_;
	void registratHandlers();
	void onStartCommand(TgBot::Message::Ptr message);
	void onText(TgBot::Message::Ptr message);
	void onCallbackQuery(TgBot::CallbackQuery::Ptr query);
	void handleMainMenuButton(TgBot::Message::Ptr message);
	void handleAddedWord(TgBot::Message::Ptr message);
	void startQuiz(int64_t chatId, int64_t userId);
	bool isValidWordInput(const string& text) const;
};