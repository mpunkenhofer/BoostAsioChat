//
// Created by necator on 8/1/17.
//

#include "chat_user_manager.h"

#include "easylogging.h"

void chat_user_manager::start(chat_user_ptr u) {
    LOG(INFO) << "added connection to manager.";
    connections_.insert(u);
    u->start();
}

void chat_user_manager::stop(chat_user_ptr u) {
    auto name = u->name();
    u->stop();
    connections_.erase(u);
    LOG(INFO) << "removed connection(" << name << ") from manager.";
}

void chat_user_manager::stop_all() {
    for (auto c : connections_)
        c->stop();

    connections_.clear();
}

chat_user_ptr chat_user_manager::user(const std::string &name) {
    auto it = std::find_if(connections_.begin(), connections_.end(), [&name](chat_user_ptr usr) {
        return usr->name() == name;
    });

    return (it != connections_.end()) ? *it : nullptr;
}

bool chat_user_manager::user_exists(const std::string &name) const {
    auto it = std::find_if(connections_.begin(), connections_.end(), [&name](chat_user_ptr usr) {
        return usr->name() == name;
    });

    return it != connections_.end();
}

std::vector<std::string> chat_user_manager::user_list() const {
    std::vector<std::string> ret;
    
    std::transform(connections_.begin(), connections_.end(), std::back_inserter(ret),
                   [](chat_user_ptr c) {
                       return c->name();
                   });
    
    return ret;
}






