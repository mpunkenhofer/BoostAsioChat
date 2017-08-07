//
// Created by necator on 8/1/17.
//

#include <iostream>
#include "chat_server.h"

#include "chat_user.h"

using boost::asio::ip::tcp;

chat_server::chat_server(boost::asio::io_service &io_service, const tcp::endpoint &endpoint) :
        io_service_(io_service),
        acceptor_(io_service, endpoint),
        manager_() {
}

void chat_server::start() {
    std::cout << "starting chat server...";
    std::cout << "listing on port: " << endpoint().port();;

    do_accept();
    io_service_.run();
}

void chat_server::do_accept() {
    chat_user_ptr new_connection = std::make_shared<chat_user>(chat_user(io_service_, manager_));

    std::cout << "waiting for a new connection...";

    acceptor_.async_accept(new_connection->socket(), [&](const boost::system::error_code &ec) {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!acceptor_.is_open())
            return;

        if (!ec) {
            std::cout << "accepted new connection.";
            manager_.start(new_connection);
        }

        do_accept();
    });
}

const boost::asio::ip::tcp::endpoint chat_server::endpoint() const {
    return acceptor_.local_endpoint();
}

void chat_server::create_channel(const std::string &id) {
    if (!id.empty()) {
        auto it = channels_.find(id);

        if(it != channels_.end()) {

        } else {
            std::cout << "a channel with the name: " << id << " exists already!";
        }
    }
}

void chat_server::remove_channel(const std::string &id) {

}

