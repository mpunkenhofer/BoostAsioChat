//
// Created by necator on 8/1/17.
//

#include <iostream>
#include "easylogging++.h"
#include "chat_user.h"

#include "chat_channel.h"
#include "chat_user_manager.h"

chat_user::chat_user(boost::asio::io_service &io_service, chat_server& server, chat_user_manager &cm) :
        io_service_(io_service),
        socket_(io_service),
        server_(server),
        manager_(cm) {

}

boost::asio::ip::tcp::socket &chat_user::socket() {
    return socket_;
}

void chat_user::start() {
    do_read_header();
}

void chat_user::stop() {
    io_service_.post([this]() {

        for (auto c : channels_)
            c->leave(shared_from_this());

        socket_.close();
    });
}

void chat_user::write(const message &msg) {
    io_service_.post([this, msg]() {
        auto write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);

        if (!write_in_progress)
            do_write();
    });
}

void chat_user::do_read_header() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_message_.data(), message::header_size()),
                            [this, self](const boost::system::error_code &ec, std::size_t s) {
                                if (!ec && read_message_.decode()) {
                                    do_read_target();
                                } else {
                                    manager_.stop(shared_from_this());
                                }
                            });
}

void chat_user::do_read_target() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_message_.target_begin(), message::target_size),
                            [this, self](const boost::system::error_code &ec, std::size_t s) {
                                if (!ec) {
                                    do_read_body();
                                } else {
                                    manager_.stop(shared_from_this());
                                }
                            });
}

void chat_user::do_read_body() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_message_.content_begin(), read_message_.content_size()),
                            [this, self](const boost::system::error_code &ec, std::size_t s) {
                                if (!ec) {

                                    do_read_header();
                                } else {
                                    manager_.stop(shared_from_this());
                                }
                            });
}

void chat_user::do_write() {
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
                             boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().total_size()),
                             [this, self](const boost::system::error_code &ec, std::size_t s) {
                                 if (!ec) {
                                     write_msgs_.pop_front();

                                     if (!write_msgs_.empty())
                                         do_write();

                                 } else {
                                     manager_.stop(shared_from_this());
                                 }
                             });
}

std::string chat_user::name(const std::string &n) {
    if (!n.empty() && n.size() > 2) {
        LOG(INFO) << "user: " << name_ << " will be now known as: " << n;
        name_ = n;
    }

    return name_;
}

std::vector<std::string> chat_user::joined_channels() const {
    std::vector<std::string> ret;

    std::transform(channels_.begin(), channels_.end(), std::back_inserter(ret), [](chat_channel_ptr u) {
        return u->name();
    });

    return ret;
}

bool chat_user::is_joined(chat_channel_ptr c) {
    auto it = channels_.find(c);

    return it != channels_.end();
}



