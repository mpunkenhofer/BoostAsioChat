//
// Created by necator on 8/1/17.
//

#include "chat_server.h"

#include <iostream>
#include <boost/algorithm/string.hpp>

#include "chat_channel.h"
#include "chat_message.h"

#include "easylogging++.h"

#define SHARED_PTR_DEBUG

using boost::asio::ip::tcp;

chat_server::chat_server(boost::asio::io_service &io_service, const tcp::endpoint &endpoint) :
        io_service_(io_service),
        acceptor_(io_service, endpoint),
        manager_() {

}

void chat_server::start() {
    LOG(INFO) << "starting chat server...";
    LOG(INFO) << "listing on port: " << endpoint().port();

    do_accept();
}

void chat_server::stop() {
    LOG(INFO) << "stopping the server...";
    acceptor_.close();
    manager_.stop_all();
}


void chat_server::do_accept() {
#ifdef SHARED_PTR_DEBUG
    chat_user_ptr new_connection = std::shared_ptr<chat_user>(new chat_user(io_service_, *this, manager_),
        [](chat_user* u) {
            LOG(INFO) << "DELETING user: " << u->name() << "!!!";
            delete u;
        });
#else
    chat_user_ptr new_connection = std::make_shared<chat_user>(chat_user(io_service_, *this, manager_));
#endif

    LOG(INFO) << "waiting for a new connection...";

    acceptor_.async_accept(new_connection->socket(), [this, new_connection](const boost::system::error_code &ec) {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!acceptor_.is_open())
            return;

        if (!ec) {
            LOG(INFO) << "new connection ... get nickname";

            boost::asio::async_read(new_connection->socket(),
                                    boost::asio::buffer(nick_buffer_, nick_buffer_.size()),
                                    [this, new_connection](const boost::system::error_code &ec,
                                                           std::size_t s __attribute__((unused))) {
                                        if(!ec) {
                                            std::string name(nick_buffer_.begin(), nick_buffer_.begin() +
                                                    std::min(std::strlen(nick_buffer_.begin()), nick_buffer_.size()));
                                            if(valid_id(name)) {
                                                LOG(INFO) << name << " is a valid name.";
                                                new_connection->name(name);
                                                new_connection->write(chat_message("server",
                                                                                   name,
                                                                                   "valid",
                                                                                   chat_message_type::status));
                                                manager_.start(new_connection);
                                            }
                                            else {
                                                LOG(INFO) << name << " is not valid!";
                                                new_connection->write(chat_message("server",
                                                                                   "target",
                                                                                   name +
                                                                                           " is not a valid name.",
                                                                                   chat_message_type::status));
                                                io_service_.post([new_connection](){ new_connection->close_socket(); });
                                            }
                                        } else {
                                            LOG(ERROR) << "Error: " << ec.message();
                                            new_connection->close_socket();
                                        }
                                    });
        }

        do_accept();
    });
}

const boost::asio::ip::tcp::endpoint chat_server::endpoint() const {
    return acceptor_.local_endpoint();
}

chat_channel_ptr chat_server::create_channel(const std::string &id) {
    if (!id.empty()) {
        auto it = channels_.find(id);

        if(it == channels_.end()) {
#ifdef SHARED_PTR_DEBUG
            channels_[id] = std::shared_ptr<chat_channel>(new chat_channel(*this, manager_, id),
                [](chat_channel* c) {
                    LOG(INFO) << "DELETING channel: " << c->name() << "!!!";
                    delete c;
                });
#else
            channels_[id] = std::make_shared<chat_channel>(chat_channel(*this, manager_, id));
#endif
            return channels_[id];
        } else {
            LOG(WARNING) << "a channel with the name: " << id << " exists already!";
            return nullptr;
        }
    }

    return nullptr;
}

bool chat_server::remove_channel(chat_channel_ptr c) {
    if(c) {
        return remove_channel(c->name());
    }

    return false;
}

bool chat_server::remove_channel(const std::string &id) {
    if(!id.empty()) {
        auto it = channels_.find(id);

        if(it != channels_.end()) {
            //it->second->leave_all(); //make sure to remove all users from the channel
            //no need to call it here since it will be called only when last user leaves the channel 
            channels_.erase(it);

            LOG(INFO) << "channel: " << id << " removed!";
        } else {
            LOG(WARNING) << "a channel with the name: " << id << " does not exist!";
            return false;
        }
    }

    return true;
}

std::vector<std::string> chat_server::channel_list() const {
    std::vector<std::string> ret;

    std::transform(channels_.begin(), channels_.end(), std::back_inserter(ret),
                   [](const std::pair<std::string, chat_channel_ptr>& c) {
                       return c.first;
                   });

    return ret;
}

std::vector<std::string> chat_server::user_list() const {
    return manager_.user_list();
}


chat_channel_ptr chat_server::channel(const std::string &id) {
    auto it = channels_.find(id);

    return it != channels_.end() ? it->second : nullptr;
}

chat_user_ptr chat_server::user(const std::string &id) {
    return manager_.user(id);
}

bool chat_server::valid_id(const std::string &id) const {
    if(id.empty())
        return false;
    
    auto it = std::find_if(id.begin(), id.end(), [](const char c) -> bool { return !isalnum(c); });
    
    if(!isalpha(id[0]) || it != id.end())
        return false;
    
    auto channel = channels_.find(id);
    auto user = manager_.user_exists(id);

    return (user || (channel != channels_.end()) || id.size() < 3) ? false : true;
}

void chat_server::handle_message(chat_message msg, chat_user_ptr user) {
    LOG(INFO) << "user: " << user->name() << "; msg: " << msg;

    if(msg.type() == chat_message_type::command)
        do_command(msg, user);
    else {
        auto target = msg.target();
        auto chan = channel(target);

        if(chan && user->is_joined(chan)) {
            chan->publish(chat_message(user->name(), target, msg.content()));
            return;
        }

        auto tell_target = manager_.user(target);

        if(tell_target) {
            tell_target->write(chat_message(user->name(), target, msg.content()));
            return;
        }

        LOG(WARNING) << "invalid target: " << target << "; no such channel or user!";

        user->write(chat_message("server",
                                 user->name(),
                                 "No channel or user with the name: " + target,
                                 chat_message_type::status));
    }
}

void chat_server::do_command(const chat_message &msg, chat_user_ptr user) {
    std::vector<std::string> tokens;
    auto content = msg.content();

    boost::split(tokens, content, boost::is_any_of("\t "));

    if(tokens.empty())
        return;

    auto command = tokens[0];
    auto arguments = std::vector<std::string>(tokens.begin() + 1, tokens.end());

    if(command == "/join" || command == "/j") {
        if(arguments.empty()) {
            user->write(chat_message("server", user->name(), "Arguments empty.", chat_message_type::status));
            return;
        }

        auto arg = arguments[0];
        auto chan = channel(arg);

        if(chan)
            chan->join(user);
        else if(valid_id(arg)) {
            auto created_chan = create_channel(arg);
            created_chan->join(user);
        }

        return;
    }

    if(command == "/leave") {
        if(arguments.empty()) {
            user->write(chat_message("server", user->name(), "Arguments empty.", chat_message_type::status));
            return;
        }

        auto arg = arguments[0];
        auto chan = channel(arg);

        if(chan && user->is_joined(chan)) {
            chan->leave(user);
        } else {
            // TODO could send user error msg
            LOG(WARNING) << user->name() << " can't leave channel he is not part of. (" << arg << ")";
        }

        return;
    }

    if(command == "/nick") {
        if(arguments.empty()) {
            user->write(chat_message("server", user->name(), "Arguments empty.", chat_message_type::status));
            return;
        }

        auto arg = arguments[0];

        if(valid_id(arg))
            user->name(arg);
        else {
            LOG(WARNING) << "id: " << arg << " already in use!";
            user->write(chat_message("server", user->name(), "Name is not valid.", chat_message_type::status));
        }

        return;
    }

    LOG(WARNING) << "unknown command. (" << user->name() << "); " << msg;
    user->write(chat_message("server", user->name(), " unknown command.", chat_message_type::status));
}