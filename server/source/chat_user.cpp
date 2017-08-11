//
// Created by necator on 8/1/17.
//

#include "chat_user.h"

#include <iostream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "easylogging++.h"

#include "chat_server.h"
#include "chat_channel.h"
#include "chat_user_manager.h"

#include <boost/asio/error.hpp>

chat_user::chat_user(boost::asio::io_service &io_service, chat_server& server, chat_user_manager &cm) :
        io_service_(io_service),
        socket_(io_service),
        ping_timer_(io_service, boost::posix_time::seconds(ping_frequency_s_)),
        current_ping_retries_(0),
        server_(server),
        manager_(cm),
        name_(chat_message::source_max_length, ' '){
    //auto uuid = boost::uuids::random_generator()();
    //std::stringstream ss;
    //ss << uuid;
    // = ss.str().substr(0, chat_message::source_max_length);
    //LOG(INFO) << "new user: " << name_;
}

boost::asio::ip::tcp::socket &chat_user::socket() {
    return socket_;
}

void chat_user::start() {
    ping();
    do_read_header();
}

void chat_user::stop() {
    leave_all_channels();
    ping_timer_.cancel();
}

void chat_user::close_socket() {
    if (socket_.is_open()) {
        socket_.shutdown(socket_.shutdown_both);
        socket_.close();
    }
}

void chat_user::write(chat_message msg) {
    io_service_.post([this, msg]() {
        auto write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);

        if (!write_in_progress)
            do_write();
    });
}

void chat_user::do_read_header() {
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
                                        manager_.stop(shared_from_this());

                                    inbound_data_.resize(inbound_data_size);

                                    do_read_message();
                                } else {
                                    LOG(ERROR) << "Error: " << ec.message();
                                    manager_.stop(shared_from_this());
                                }
                            });
}

void chat_user::do_read_message() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(inbound_data_, inbound_data_.size()),
                            [this, self](const boost::system::error_code &ec,
                                         std::size_t s __attribute__ ((unused))) {
                                if (!ec) {
                                    try {
                                        auto msg = deserialize_chat_message(inbound_data_);
                                        if (msg.type() == chat_message_type::status && msg.content() == "pong") {
                                            LOG(INFO) << name_ << ": received pong from client.";
                                            pong_received_ = true;
                                        } else
                                            server_.handle_message(std::move(msg), shared_from_this());
                                    } catch(const boost::archive::archive_exception& e) {
                                        LOG(ERROR) << "Error: Failed to decode msg: " << e.what();
                                    }
                                    do_read_header();
                                } else {
                                    LOG(ERROR) << "Error: " << ec.message();
                                    manager_.stop(shared_from_this());
                                }
                            });
}

void chat_user::do_write() {
    auto self(shared_from_this());

    boost::asio::async_write(socket_,
                             write_msgs_.front().generate_buffers(),
                             [this, self](const boost::system::error_code &ec,
                                          std::size_t s __attribute__ ((unused))) {
                                 if (!ec) {
                                     write_msgs_.pop_front();

                                     if (!write_msgs_.empty())
                                         do_write();

                                 } else {
                                     LOG(ERROR) << "Error: " << ec.message();
                                     manager_.stop(shared_from_this());
                                 }
                             });
}

void chat_user::ping() {
    LOG(INFO) << "Sending client(" << name_ << ") a ping.";

    write(chat_message("server", name_, "ping", chat_message_type::status));

    auto self(shared_from_this());

    ping_timer_.async_wait([this, self](const boost::system::error_code& ec) {
        if(!ec) {
            if (pong_received_) {
                current_ping_retries_ = 0;
                pong_received_ = false;
                ping_timer_.expires_at(ping_timer_.expires_at() + boost::posix_time::seconds(ping_frequency_s_));
                ping();
            } else if (current_ping_retries_ < ping_retry_max_) {
                current_ping_retries_++;
                ping_timer_.expires_at(ping_timer_.expires_at() + boost::posix_time::seconds(ping_frequency_s_ / 2));

                LOG(INFO) << "Ping retry " << std::to_string(current_ping_retries_)
                          << " of " << std::to_string(ping_retry_max_) << ".";

                ping();
            } else {
                LOG(INFO) << name_ << ": Ping Timeout!";

                leave_all_channels();
                close_socket();
            }
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

char* chat_user::name_data() {
    return &name_[0];
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

    auto boolean = it != channels_.end();

    return boolean;
}

void chat_user::leave_all_channels() {
    LOG(INFO) << "User: " << name_ << " is leaving all channels.";

    //make copy so the channel can safely remove itself from the users channel list
    auto chans = channels_;

    for (chat_channel_ptr c : chans)
        c->leave(shared_from_this());

    //channels_ ought to be empty here.
    channels_.clear();
}


