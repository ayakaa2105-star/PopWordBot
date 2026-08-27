#include <tgbot/tgbot.h>
#include <iostream>
#include <string>
#include "bot/BotApp.h"
using namespace std;
int main() {
	string token = ("8311647509:AAHcKklRVZpyuz4zUp9wjOKccncIzs4l9SA");
	cout << "bot is running..." << endl;
	BotApp app(token);
	app.run();
	return 0;
}