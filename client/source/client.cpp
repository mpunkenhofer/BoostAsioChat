//
// Created by necator on 7/31/17.
//

#include "chat_client.h"

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <thread>

//#define ELPP_DISABLE_INFO_LOGS
#include "easylogging.h"
INITIALIZE_EASYLOGGINGPP

void init_logger();

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cout << "Usage: ./Client <host> <port>\n";
        return 1;
    }

    init_logger();

    try {
        LOG(INFO) << "starting chat client...";

        boost::asio::io_service io_service;
        boost::asio::ip::tcp::resolver resolver(io_service);

        auto endpoint_iterator = resolver.resolve({argv[1], argv[2]});

        chat_client client(io_service, endpoint_iterator);

        std::cout << "In order to successfully connect to the server you have to pick a vaild nick name.\n";

        std::string nick;
        do {
            std::cout << "Nickname: ";
            std::cin >> nick;
        }
        while(!client.connect(nick));

        std::thread t([&io_service] { io_service.run(); });

        std::cout << "To change the target you are talking to: /t <target>\n";
        std::string active_target;
        char line[chat_message::content_max_length + 1];

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        //std::cin.clear();

        while (std::cin.getline(line, chat_message::content_max_length + 1).good()) {
            if (std::strcmp(line, "/quit") == 0 || std::strcmp(line, "/q") == 0)
                break;

            if (std::strncmp(line, "/target", std::strlen("/target")) == 0 ||
                    std::strncmp(line, "/t", std::strlen("/t")) == 0) {
                std::vector<std::string> tokens;
                boost::split(tokens, line, boost::is_any_of("\t "));

                if (tokens.size() > 1) {
                    active_target = tokens[1];
                    std::cout << "You are now talking to: " << active_target << '\n';
                } else {
                    std::cout << "Currently talking to: " << active_target << '\n';
                }
            } else if (line[0] == '/') {
                chat_message msg(nick, active_target, std::string(line, line + std::strlen(line)), chat_message_type::command);
                client.write(msg);
            } else {
                chat_message msg(nick, active_target, std::string(line, line + std::strlen(line)));
                client.write(msg);
            }
        }

        client.close();
        t.join();

    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}

void init_logger() {
    el::Configurations defaultConf;

    defaultConf.set(el::Level::Info,
                    el::ConfigurationType::Format, "%datetime %level [%func]: %msg");
    defaultConf.set(el::Level::Warning,
                    el::ConfigurationType::Format, "%datetime %level [%func]: %msg");
    defaultConf.set(el::Level::Error,
                    el::ConfigurationType::Format, "%datetime %level [%func]: %msg");

    el::Loggers::reconfigureLogger("default", defaultConf);
}