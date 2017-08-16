//
// Created by necator on 8/4/17.
//

#include <boost/asio.hpp>

#include "chat_client.h"

//#define ELPP_DISABLE_INFO_LOGS
#include "easylogging.h"

chat_client::chat_client(boost::asio::io_service &io_service,
                         boost::asio::ip::tcp::resolver::iterator endpoint_iterator) :
        io_service_(io_service),
        socket_(io_service),
        write_strand_(io_service),
        endpoint_(std::move(endpoint_iterator)){
    //do_connect(endpoint_iterator);
}

void chat_client::close() {
    io_service_.post([this](){
        if(socket_.is_open()) {
            socket_.shutdown(socket_.shutdown_both);
            socket_.close();
        }
    });
}

void chat_client::write(chat_message &msg) {
    io_service_.post(write_strand_.wrap([this, msg]() {
        auto write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);

        if (!write_in_progress)
            do_write();
    }));
}

bool chat_client::connect(const std::string& nickname) {
    try {
        std::array<char, chat_message::source_max_length> nick_buffer;
        auto len = std::min(chat_message::source_max_length - 1, nickname.size());

        std::strncpy(nick_buffer.begin(), nickname.c_str(), len);
        nick_buffer[len] = '\0';

        boost::asio::connect(socket_, endpoint_);
        boost::asio::write(socket_, boost::asio::buffer(nick_buffer, nick_buffer.size()));

        std::array<char, chat_message::header_length> header;
        boost::asio::read(socket_, boost::asio::buffer(header, header.size()));

        std::istringstream is(std::string(header.begin(), header.end()));
        std::size_t chat_msg_size = 0;

        if (!(is >> std::hex >> chat_msg_size)) {
            return false;
        }

        std::vector<char> message_data(chat_msg_size);
        boost::asio::read(socket_, boost::asio::buffer(message_data, message_data.size()));

        auto msg = deserialize_chat_message(message_data);

        if(msg.type() == chat_message_type::status && msg.content() == "valid") {
            do_read_header(); //start
            return true;
        }

        if(msg.type() == chat_message_type::status){
            std::cout << "Server: " << msg.content() << '\n';
        }
    } catch(const std::exception& e) {
        LOG(ERROR) << "Error: " << e.what();
    }

    return false;
}

//void chat_client::do_connect(boost::asio::ip::tcp::resolver::iterator endpoint) {
//    LOG(INFO) << "connect to server...";
//
//    boost::asio::async_connect(socket_,
//                               endpoint,
//                               [this](const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator) {
//                                   if (!ec)
//                                       do_read_header();
//                               });
//}

void chat_client::do_read_header() {
    LOG(INFO) << "waiting for a msg...";

    boost::asio::async_read(socket_,
                            boost::asio::buffer(inbound_header_, inbound_header_.size()),
                            [this](const boost::system::error_code &ec,
                                         std::size_t s __attribute__ ((unused))) {
                                if (!ec) {
                                    std::istringstream is(std::string(inbound_header_.begin(), inbound_header_.end()));
                                    std::size_t inbound_data_size = 0;

                                    if (!(is >> std::hex >> inbound_data_size))
                                        error_handler();

                                    inbound_data_.resize(inbound_data_size);

                                    do_read_message();
                                } 
                                else if(ec == boost::asio::error::operation_aborted)
                                    LOG(INFO) << "operation aborted.";
                                else {
                                    LOG(ERROR) << "Error: " << ec.message();
                                    error_handler();
                                }
                            });
}

void chat_client::do_read_message() {
    boost::asio::async_read(socket_,
                            boost::asio::buffer(inbound_data_, inbound_data_.size()),
                            [this](const boost::system::error_code &ec,
                                         std::size_t s __attribute__ ((unused))) {
                                if (!ec) {
                                    handle_message(deserialize_chat_message(inbound_data_));

                                    do_read_header();
                                } 
                                else if(ec == boost::asio::error::operation_aborted)
                                    LOG(INFO) << "operation aborted.";
                                else {
                                    LOG(ERROR) << "Error: " << ec.message();
                                    error_handler();
                                }
                            });
}

void chat_client::do_write() {
    boost::asio::async_write(socket_,
                             write_msgs_.front().generate_buffers(),
                             write_strand_.wrap([this](const boost::system::error_code &ec,
                                                             std::size_t s __attribute__ ((unused))) {
                                 if (!ec) {
                                     write_msgs_.pop_front();

                                     if (!write_msgs_.empty())
                                         do_write();

                                 } else {
                                     LOG(ERROR) << "Error: " << ec.message();
                                     error_handler();
                                 }
                             }));
}

void chat_client::handle_message(chat_message msg) {
    if(msg.type() == chat_message_type::text)
        print(std::move(msg));
    else if(msg.type() == chat_message_type::status && msg.content() == "ping") {
        LOG(INFO) << "server sent a ping ... replaying with a pong.";

        chat_message pong("source", "server", "pong", chat_message_type::status);

        write(pong);
    }
    else if(msg.type() == chat_message_type::status) {
        std::cout << "Server: " << msg.content() << '\n';
    }
    else {
        LOG(INFO) << "client can't handle this msg: " << msg;
    }
}

void chat_client::print(chat_message msg) {
    std::cout << msg.target() << "::" << msg.source() << ": " << msg.content() << '\n';
}

void chat_client::error_handler() {
    LOG(ERROR) << "an error occurred, closing connection.";
    close();
}