//
// Created by necator on 8/1/17.
//

#ifndef BOOSTCHAT_CHAT_SERVER_H
#define BOOSTCHAT_CHAT_SERVER_H

#include <boost/asio.hpp>
#include <deque>

#include "chat_user_manager.h"

class chat_message;

class chat_channel;

using chat_channel_ptr = std::shared_ptr<chat_channel>;

class chat_server {
public:
    explicit chat_server(boost::asio::io_service &io_service,
                const boost::asio::ip::tcp::endpoint &endpoint =
                boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 666));

    void start();
    void stop();

    const boost::asio::ip::tcp::endpoint endpoint() const;

    bool remove_channel(const std::string &id);
    bool remove_channel(chat_channel_ptr c);

    chat_channel_ptr create_channel(const std::string &id);

    std::vector<std::string> channel_list() const;
    std::vector<std::string> user_list() const;

    chat_channel_ptr channel(const std::string& id);
    chat_user_ptr user(const std::string& id);

    void handle_message(chat_message msg, chat_user_ptr user);

    bool valid_id(const std::string &id) const;
private:
    boost::asio::io_service &io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::array<char, chat_message::source_max_length> nick_buffer_;

    chat_user_manager manager_;

    std::map<std::string, chat_channel_ptr> channels_;

    void do_accept();

    void do_command(const chat_message &msg, chat_user_ptr user);
};


#endif //BOOSTCHAT_CHAT_SERVER_H
