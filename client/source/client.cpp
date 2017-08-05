//
// Created by necator on 7/31/17.
//

#include "chat_client.h"

#include <iostream>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <string>
#include <thread>

#include "message.h"

int main()
{
  try {
    std::cout << "Starting chat client...\n";

    boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver(io_service);

    auto endpoint_iterator = resolver.resolve({"necator.net", "6325"});

    chat_client client(io_service, endpoint_iterator);

    std::thread t([&io_service] { io_service.run(); });

    std::string active_target("-");
    char line[message::max_length + 1];

    std::cout << active_target << ": ";

    while(std::cin.getline(line, message::max_length + 1)) {
      if(std::strcmp(line, "/quit") == 0 || std::strcmp(line, "/q") == 0)
        break;
      else if(std::strncmp(line, "/channel", std::strlen("/channel")) == 0 || std::strncmp(line, "/c", std::strlen("/c")) == 0) {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of("\t "));

        if(tokens.size() > 1)
          active_target = tokens[1];
      }
      else if(line[0] == '/') {
        message msg(std::string(line, line + std::strlen(line)), message::message_type::command);
        client.write(msg);
      }
      else {
        message msg(std::string(line, line + std::strlen(line)), active_target);
        client.write(msg);
      }

      std::cout << active_target << ": ";
    }

    client.close();
    t.join();

  } catch(std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}