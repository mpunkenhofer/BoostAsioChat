//
// Created by necator on 7/31/17.
//

#include <iostream>
#include <boost/asio.hpp>

#include "chat_server.h"

int main(int argc, char** argv)
{
  if(argc != 2) {
    std::cout << "Usage: ./server <port>\n";
    return 1;
  }

  try {
    std::cout << "Starting chat server...\n";

    boost::asio::io_service io_service;

    unsigned short port = static_cast<unsigned short>(std::atoi(argv[1]));
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

    chat_server server(io_service, endpoint);

    std::cout << "Chat server running and listing on port: " << server.endpoint().port() << "\n";

    server.start();

  } catch(std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}