//
// Created by necator on 8/4/17.
//

#ifndef BOOSTCHAT_CHAT_CHANNEL_H
#define BOOSTCHAT_CHAT_CHANNEL_H

#include <string>
#include <set>
#include <vector>
#include <memory>

class chat_message;

class chat_user;

class chat_server;

class chat_user_manager;

using chat_user_ptr = std::shared_ptr<chat_user>;

class chat_channel : public std::enable_shared_from_this<chat_channel> {
public:
    chat_channel(chat_server &server, chat_user_manager &manager, const std::string &name);

    void publish(const chat_message &msg);

    std::string name() const;

    std::vector<std::string> user_list() const;

    void join(chat_user_ptr c);

    void leave(chat_user_ptr c);

    void leave_all();

private:
    chat_server &server_;
    chat_user_manager &manager_;
    std::string name_;

    std::set<chat_user_ptr> users_;
};

using chat_channel_ptr = std::shared_ptr<chat_channel>;

#endif //BOOSTCHAT_CHAT_CHANNEL_H
