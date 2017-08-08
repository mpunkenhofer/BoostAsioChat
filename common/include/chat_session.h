//
// Created by necator on 8/8/17.
//

#ifndef BOOSTASIOCHAT_CHAT_SESSION_H
#define BOOSTASIOCHAT_CHAT_SESSION_H


#include <boost/asio/impl/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <deque>


#include "chat_message.h"

class chat_session;

using chat_session_ptr = std::shared_ptr<chat_session>;

class chat_session : public std::enable_shared_from_this<chat_session> {
public:
    chat_session(boost::asio::io_service&,
                 std::function<void(const chat_message& msg, chat_session_ptr)> message_handler,
                 std::function<void(chat_session_ptr)> error_handler);

    void write(chat_message &msg);

    boost::asio::ip::tcp::socket& socket();

    void start_reading();
    void close_socket();

    virtual ~chat_session();
protected:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;
private:
    std::function<void(const chat_message& msg, chat_session_ptr)> message_handler_;
    std::function<void(chat_session_ptr)> error_handler_;
    boost::asio::io_service::strand write_strand_;
    std::deque<chat_message> write_msgs_;


    std::array<char, chat_message::header_length> inbound_header_;
    std::vector<char> inbound_data_;

    void do_read_header();
    void do_read_message();
    void do_write();
};


#endif //BOOSTASIOCHAT_CHAT_SESSION_H
