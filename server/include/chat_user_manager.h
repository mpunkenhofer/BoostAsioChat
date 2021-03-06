//
// Created by necator on 8/1/17.
//

#ifndef BOOSTCHAT_CONNECTION_MANAGER_H
#define BOOSTCHAT_CONNECTION_MANAGER_H

#include <set>

#include "chat_user.h"

class chat_user_manager {
public:
    chat_user_manager() = default;

    ~chat_user_manager() = default;

    chat_user_manager(const chat_user_manager &) = delete;

    chat_user_manager &operator=(const chat_user_manager &) = delete;

    chat_user_manager(chat_user_manager&&) = delete;

    chat_user_manager& operator=(chat_user_manager&&) = delete;

    void start(chat_user_ptr c);

    void stop(chat_user_ptr c);

    void stop_all();

    chat_user_ptr user(const std::string &name);
    bool user_exists(const std::string& name) const;
    std::vector<std::string> user_list() const;
private:
    std::set<chat_user_ptr> connections_;
};


#endif //BOOSTCHAT_CONNECTION_MANAGER_H
