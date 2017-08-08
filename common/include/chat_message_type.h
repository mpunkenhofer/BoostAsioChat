//
// Created by necator on 8/8/17.
//

#ifndef BOOSTASIOCHAT_CHAT_MESSAGE_TYPE_H
#define BOOSTASIOCHAT_CHAT_MESSAGE_TYPE_H


#include <string>

enum class chat_message_type {
    text,
    command,
    status,
    unknown
};

int to_int(chat_message_type t);
std::string to_string(chat_message_type t);

#endif //BOOSTASIOCHAT_CHAT_MESSAGE_TYPE_H
