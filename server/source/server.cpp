//
// Created by necator on 7/31/17.
//

#include <iostream>
#include <boost/asio.hpp>

#include "chat_server.h"

int main()
{
  try {
    std::cout << "Starting chat server...\n";

    boost::asio::io_service io_service;
    chat_server server(io_service);

    std::cout << "Chat server running and listing on port: " << server.endpoint().port() << "\n";

    server.start();

  } catch(std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}