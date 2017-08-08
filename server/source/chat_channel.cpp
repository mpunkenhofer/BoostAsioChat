//
// Created by necator on 8/4/17.
//

#include <iostream>
#include "chat_channel.h"

#include "easylogging++.h"

#include "chat_user.h"
#include "chat_server.h"

chat_channel::chat_channel(chat_server &server, chat_user_manager &manager, const std::string &name) :
        server_(server),
        manager_(manager),
        name_(name) {
    LOG(INFO) << "channel: " << name_ << " is created.";
}

void chat_channel::publish(const chat_message &msg) {
    LOG(INFO) << "channel: " << name_ << "publish: " << msg;

    for (auto u : users_)
        u->write(msg);
}

std::string chat_channel::name() const {
    return name_;
}

std::vector<std::string> chat_channel::user_names() const {
    std::vector<std::string> ret;

    std::transform(users_.begin(), users_.end(), std::back_inserter(ret), [](const chat_user_ptr u) {
        return u->name();
    });

    return ret;
}

void chat_channel::join(chat_user_ptr c) {
    LOG(INFO) << "user: " << c->name() << " joined: " << name_;
    users_.insert(c);
}

void chat_channel::leave(chat_user_ptr c) {
    LOG(INFO) << "user: " << c->name() << " left: " << name_;
    users_.erase(c);

    if (users_.empty()) {
        server_.remove_channel(name_);
    }
}

void chat_channel::leave_all() {
    if(!users_.empty()) {
        LOG(INFO) << name_ << " all users are forced to leave.";

        users_.clear();

        server_.remove_channel(name_);
    }
}













