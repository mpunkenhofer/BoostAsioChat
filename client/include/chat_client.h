//
// Created by necator on 8/4/17.
//

#ifndef BOOSTCHAT_CHAT_CLIENT_H
#define BOOSTCHAT_CHAT_CLIENT_H

#include <boost/asio.hpp>
#include <deque>

#include "chat_message.h"

class chat_client;

using chat_client_ptr = std::shared_ptr<chat_client>;

class chat_client {
public:
    chat_client(boost::asio::io_service &io_service, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

    void write(chat_message &msg);
    void close();
private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;

    boost::asio::io_service::strand write_strand_;
    std::deque<chat_message> write_msgs_;


    std::array<char, chat_message::header_length> inbound_header_;
    std::vector<char> inbound_data_;

    void do_connect(boost::asio::ip::tcp::resolver::iterator e);
    void do_read_header();
    void do_read_message();
    void do_write();

    void print(const chat_message&);
    void error_handler();
};
#endif //BOOSTCHAT_CHAT_CLIENT_H
