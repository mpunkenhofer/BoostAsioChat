//
// Created by necator on 7/31/17.
//

#include <iostream>
#include <boost/asio.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <thread>

#include "chat_server.h"
#include "chat_channel.h"

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

        std::thread t([&io_service](){ io_service.run(); });

        std::string s;

        while(std::getline(std::cin, s)) {
            if(s == "/q" || s == "/quit")
                break;
            else if(s == "/users") {
                auto users = server.user_list();

                if(users.empty())
                    std::cout << "No users on the server.";
                else {
                    std::cout << "[" << users.size() << "] ";
                    std::copy(users.begin(), users.end(), std::ostream_iterator<std::string>(std::cout, " "));
                }
                std::cout << '\n';

                continue;
            }
            else if(std::strncmp(s.c_str(), "/u", std::strlen("/u")) == 0 ||
                    std::strncmp(s.c_str(), "/user", std::strlen("/user")) == 0) {
                std::vector<std::string> tokens;
                boost::split(tokens, s, boost::is_any_of("\t "));

                if(tokens.size() > 1) {
                    auto user = server.user(tokens[1]);

                    if(user) {
                        auto chans = user->joined_channels();

                        if (chans.empty())
                            std::cout << "User: \"" << user->name() << "\" has not joined any channels.";
                        else {
                            std::cout << "Channels: ";
                            std::copy(chans.begin(), chans.end(), std::ostream_iterator<std::string>(std::cout, " "));
                        }

                        std::cout << '\n';
                    } else {
                        std::cout << "User: \"" << tokens[1] << "\" not found!\n";
                    }
                }

                continue;
            }
            else if(s == "/channels") {
                auto chans = server.channel_list();

                if(chans.empty())
                    std::cout << "No channels.";
                else {
                    std::cout << "[" << chans.size() << "] ";
                    std::copy(chans.begin(), chans.end(), std::ostream_iterator<std::string>(std::cout, " "));
                }

                std::cout << '\n';

                continue;
            }
            else if(std::strncmp(s.c_str(), "/c", std::strlen("/c")) == 0 ||
                    std::strncmp(s.c_str(), "/channel", std::strlen("/channel")) == 0) {
                std::vector<std::string> tokens;
                boost::split(tokens, s, boost::is_any_of("\t "));

                if(tokens.size() > 1) {
                    auto chan = server.channel(tokens[1]);

                    if(chan) {
                        auto users = chan->user_list();

                        std::cout << "Users in #" << chan->name() << ": ";
                        std::copy(users.begin(), users.end(), std::ostream_iterator<std::string>(std::cout, " "));
                    } else {
                        std::cout << "Channel: \"" << tokens[1] << "\" not found!";
                    }

                    std::cout << '\n';
                }

                continue;
            }
        }

        server.stop();
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