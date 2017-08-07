//
// Created by necator on 7/31/17.
//

#include <iostream>
#include <boost/asio.hpp>

#include "chat_server.h"

#define _ELPP_THREAD_SAFE
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP

void init_logger();

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Usage: ./Server <port>\n";
        return 1;
    }

    init_logger();

    try {
        boost::asio::io_service io_service;

        unsigned short port = static_cast<unsigned short>(std::atoi(argv[1]));
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

        chat_server server(io_service, endpoint);

        server.start();

    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

void init_logger() {
    el::Configurations defaultConf;
    defaultConf.set(el::Level::Info,
                    el::ConfigurationType::Format, "%datetime %level [%func]: %msg");
    el::Loggers::reconfigureLogger("default", defaultConf);
}