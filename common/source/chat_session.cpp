//
// Created by necator on 8/8/17.
//

#include <boost/asio.hpp>
#include "chat_session.h"

#include "easylogging++.h"

chat_session::chat_session(boost::asio::io_service& io_service,
                           std::function<void(const chat_message& msg, chat_session_ptr)> message_handler,
                           std::function<void(chat_session_ptr)> error_handler) :
        io_service_(io_service),
        socket_(io_service),
        message_handler_(message_handler),
        error_handler_(error_handler),
        write_strand_(io_service) {
    LOG(INFO) << "new chat session";
}

void chat_session::start_reading() {
    do_read_header();
}

void chat_session::close_socket() {
    io_service_.post([this](){ socket_.close(); });
}

void chat_session::write(chat_message &msg) {
    io_service_.post(write_strand_.wrap([this, msg]() {
        auto write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(std::move(msg));

        if (!write_in_progress)
            do_write();
    }));
}

void chat_session::do_read_header() {
    LOG(INFO) << "waiting for a msg...";

    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(inbound_header_, inbound_header_.size()),
                            [this, self](const boost::system::error_code &ec,
                                         std::size_t s __attribute__ ((unused))) {
                                if (!ec) {
                                    std::istringstream is(std::string(inbound_header_.begin(), inbound_header_.end()));
                                    std::size_t inbound_data_size = 0;

                                    if (!(is >> std::hex >> inbound_data_size))
                                        error_handler_(shared_from_this());

                                    inbound_data_.resize(inbound_data_size);

                                    do_read_message();
                                } else {
                                    error_handler_(shared_from_this());
                                }
                            });
}

void chat_session::do_read_message() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(inbound_data_, inbound_data_.size()),
                            [this, self](const boost::system::error_code &ec,
                                         std::size_t s __attribute__ ((unused))) {
                                if (!ec) {
                                    message_handler_(deserialize_chat_message(inbound_data_), shared_from_this());

                                    do_read_header();
                                } else {
                                    error_handler_(shared_from_this());
                                }
                            });
}

void chat_session::do_write() {
    auto self(shared_from_this());

    boost::asio::async_write(socket_,
                             write_msgs_.front().generate_buffers(),
                             write_strand_.wrap([this, self](const boost::system::error_code &ec,
                                          std::size_t s __attribute__ ((unused))) {
                                 if (!ec) {
                                     write_msgs_.pop_front();

                                     if (!write_msgs_.empty())
                                         do_write();

                                 } else {
                                     error_handler_(shared_from_this());
                                 }
                             }));
}

chat_session::~chat_session() {
}
