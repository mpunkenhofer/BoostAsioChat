//
// Created by necator on 8/8/17.
//

#include <vector>
#include "chat_message_type.h"

int to_int(chat_message_type t) {
    switch(t) {
        case chat_message_type::command: return 0;
        case chat_message_type::text: return 1;
        case chat_message_type::status: return 2;
        case chat_message_type::unknown: return 3;
    }

    return -1;
}

std::string to_string(chat_message_type t) {
    const static std::vector<std::string> names {"command", "text", "status", "unknown"};

    return names[to_int(t)];
}