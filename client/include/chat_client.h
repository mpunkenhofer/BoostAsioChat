//
// Created by necator on 8/4/17.
//

#ifndef BOOSTCHAT_CHAT_CLIENT_H
#define BOOSTCHAT_CHAT_CLIENT_H

#include <boost/asio.hpp>
#include <deque>

#include "message.h"

class chat_client
{
public:
  chat_client(boost::asio::io_service& io_service, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

  void write(const message &msg);
  void close();
private:
  boost::asio::io_service& io_service_;
  boost::asio::ip::tcp::socket socket_;
  message read_msg_;
  std::deque<message> write_msgs_;

  void do_connect(boost::asio::ip::tcp::resolver::iterator);
  void do_read_header();
  void do_read_target();
  void do_read_body();
  void do_write();

  void print();
};


#endif //BOOSTCHAT_CHAT_CLIENT_H
