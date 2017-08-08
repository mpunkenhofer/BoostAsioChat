//
// Created by necator on 8/1/17.
//

#ifndef BOOSTCHAT_CONNECTION_H
#define BOOSTCHAT_CONNECTION_H

#include <boost/asio.hpp>
#include <memory>
#include <array>
#include <deque>
#include <set>
#include <chat_message.h>


class chat_server;

class chat_channel;

class chat_user_manager;

using chat_channel_ptr = std::shared_ptr<chat_channel>;

class chat_user : public std::enable_shared_from_this<chat_user> {
public:
    //chat_user(const chat_user&) = delete;
    //chat_user& operator=(const chat_user&) = delete;

    chat_user(boost::asio::io_service &, chat_server &, chat_user_manager &);

    void start();

    void stop();

    void write(const chat_message &msg);

    std::string name(const std::string &n = std::string(""));

    std::vector<std::string> joined_channels() const;
    bool is_joined(chat_channel_ptr c);

    boost::asio::ip::tcp::socket &socket();

    void join_channel(chat_channel_ptr);
    void leave_channel(chat_channel_ptr);
    void leave_all_channels();
    
private:
    boost::asio::io_service &io_service_;
    boost::asio::ip::tcp::socket socket_;

    chat_server& server_;
    chat_user_manager& manager_;

    std::string name_;

    std::set<chat_channel_ptr> channels_;

    std::deque<chat_message> write_msgs_;

    std::array<char, chat_message::header_length> inbound_header_;
    std::vector<char> inbound_data_;

    void do_read_header();

    void do_read_message();

    void do_write();
};

using chat_user_ptr = std::shared_ptr<chat_user>;

#endif //BOOSTCHAT_CONNECTION_H
