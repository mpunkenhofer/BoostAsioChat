//
// Created by necator on 8/4/17.
//

#include <iostream>
#include "chat_channel.h"

#include "chat_user.h"
#include "chat_server.h"

chat_channel::chat_channel(chat_server &server, chat_user_manager &manager, const std::string &name) :
        server_(server),
        manager_(manager),
        name_(name) {
}

void chat_channel::publish(const message &msg) {
    std::cout << "publish: " + msg.debug_string();

    for (auto u : users_)
        u->write(msg);
}

std::string chat_channel::name() const {
    return name_;
}

std::vector<std::string> chat_channel::user_names() const {
    return std::vector<std::string>();
}

void chat_channel::join(chat_user_ptr c) {
    users_.insert(c);
}

void chat_channel::leave(chat_user_ptr c) {
    users_.erase(c);

    if (users_.empty()) {
        server_.remove_channel(name_);
    }
}

void chat_channel::leave_all() {
    users_.clear();
}













