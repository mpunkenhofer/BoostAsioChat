//
// Created by necator on 8/1/17.
//

#include <iostream>
#include <boost/algorithm/string.hpp>
#include "chat_server.h"

#include "chat_user.h"
#include "chat_channel.h"

#include "easylogging++.h"

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
    io_service_.run();
}

void chat_server::do_accept() {
    chat_user_ptr new_connection = std::make_shared<chat_user>(chat_user(io_service_, *this, manager_));

    LOG(INFO) << "waiting for a new connection...";

    acceptor_.async_accept(new_connection->socket(), [this, new_connection](const boost::system::error_code &ec) {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!acceptor_.is_open())
            return;

        if (!ec) {
            LOG(INFO) << "accepted new connection.";
            manager_.start(new_connection);
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
            channels_[id] = std::make_shared<chat_channel>(chat_channel(*this, manager_, id));
            return channels_[id];
        } else {
            LOG(WARNING) << "a channel with the name: " << id << " exists already!";
            return nullptr;
        }
    }

    return nullptr;
}

bool chat_server::remove_channel(const std::string &id) {
    if(!id.empty()) {
        auto it = channels_.find(id);

        if(it != channels_.end()) {
            LOG(INFO) << "channel: " << id << " deleted!";

            it->second->leave_all(); //make sure to remove all users from the channel
            channels_.erase(it);
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

chat_channel_ptr chat_server::channel(const std::string &id) {
    auto it = channels_.find(id);

    return it != channels_.end() ? it->second : nullptr;
}

bool chat_server::unused_id(const std::string &id) const {
    auto channel = channels_.find(id);
    auto user = manager_.user_exists(id);

    return (user || (channel != channels_.end())) ? false : true;
}

void chat_server::handle_message(const message &msg, chat_user_ptr user) {
    LOG(INFO) << "user: " << user->name() << "; msg: " << msg;

    if(msg.type() == message::message_type::command)
        do_command(msg, user);
    else {
        auto target = msg.target();
        auto chan = channel(target);

        if(chan && user->is_joined(chan)) {
            chan->publish(message(msg.content(), user->name().substr(0,std::min(user->name().size(),
                                                                                message::target_size - 1))));
            return;
        }

        auto tell_target = manager_.user(target);

        if(tell_target) {
            tell_target->write(msg);
            return;
        }

        //TODO send user error msg
        LOG(WARNING) << "invalid target: " << target << "; no such channel or user!";
    }
}

void chat_server::do_command(const message &msg, chat_user_ptr user) {
    std::vector<std::string> tokens;
    auto content = msg.content();

    boost::split(tokens, content, boost::is_any_of("\t "));

    if(tokens.empty())
        return;

    auto command = tokens[0];
    auto arguments = std::vector<std::string>(tokens.begin() + 1, tokens.end());

    if(command == "/join") {
        if(arguments.empty()) {
            //TODO send error msg to user
            return;
        }

        auto arg = arguments[0];
        auto chan = channel(arg);

        if(chan)
            user->join_channel(chan);
        else if(unused_id(arg)) {
            auto created_chan = create_channel(arg);
            user->join_channel(created_chan);
        }

        return;
    }

    if(command == "/leave") {
        if(arguments.empty()) {
            //TODO send error msg to user
            return;
        }

        auto arg = arguments[0];
        auto chan = channel(arg);

        if(chan && user->is_joined(chan)) {
            user->leave_channel(chan);
        } else {
            // TODO could send user error msg
            LOG(WARNING) << user->name() << " can't leave channel he is not part of. (" << arg << ")";
        }

        return;
    }

    if(command == "/nick") {
        if(arguments.empty()) {
            //TODO send error msg to user
            return;
        }

        auto arg = arguments[0];

        if(unused_id(arg))
            user->name(arg);
        else {
            // TODO send user error msg
            LOG(WARNING) << "id: " << arg << " already in use!";
        }

        return;
    }

    LOG(WARNING) << "unknown command. (" << user->name() << "); " << msg;
}