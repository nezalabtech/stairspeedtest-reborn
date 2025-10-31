#ifndef TELEGRAM_H_INCLUDED
#define TELEGRAM_H_INCLUDED

#include <string>
#include <vector>
#include "nodeinfo.h"

// Telegram notification functions
bool sendTelegramPhoto(const std::string &bot_token, const std::string &chat_id, 
                      const std::string &photo_path, const std::string &caption = "");
std::string generateResultCaption(const std::vector<nodeInfo> &nodes, const std::string &group_name = "");
bool isTelegramEnabled();
std::string getTelegramBotToken();
std::string getTelegramChatId();
bool getTelegramSendCaption();

#endif // TELEGRAM_H_INCLUDED