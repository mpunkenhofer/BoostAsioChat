//
// Created by necator on 8/4/17.
//

#include "chat_client.h"

#include <iostream>

chat_client::chat_client(boost::asio::io_service& io_service,
                         boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
  : io_service_(io_service),
    socket_(io_service)
{
  do_connect(endpoint_iterator);
}

void chat_client::write(const message &msg)
{
  io_service_.post([this, msg]() {
    auto write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);

    if(!write_in_progress)
      do_write();
  });

}

void chat_client::close()
{
  io_service_.post([this](){ socket_.close(); });
}

void chat_client::do_connect(boost::asio::ip::tcp::resolver::iterator e)
{
  boost::asio::async_connect(socket_,
                             e,
                             [this](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator) {
    if(!ec)
      do_read_header();
  });
}

void chat_client::do_read_header()
{
  boost::asio::async_read(socket_,
                          boost::asio::buffer(read_msg_.data(), message::header_size()),
                          [this](const boost::system::error_code& ec, std::size_t s) {
                            if(!ec && read_msg_.decode()) {
                              do_read_target();
                            } else {
                              socket_.close();
                            }
                          });
}

void chat_client::do_read_target()
{
  boost::asio::async_read(socket_,
                          boost::asio::buffer(read_msg_.target_begin(), message::target_size),
                          [this](const boost::system::error_code& ec, std::size_t s) {
                            if(!ec) {
                              do_read_body();
                            } else {
                              socket_.close();
                            }
                          });
}

void chat_client::do_read_body()
{
  boost::asio::async_read(socket_,
                          boost::asio::buffer(read_msg_.content_begin(), read_msg_.content_size()),
                          [this](const boost::system::error_code& ec, std::size_t s) {
                            if(!ec) {
                              print();
                              do_read_header();
                            } else {
                              socket_.close();
                            }
                          });
}

void chat_client::print()
{
  std::cout << read_msg_.target() << ": " << read_msg_.content() << '\n';
}

void chat_client::do_write()
{
  boost::asio::async_write(socket_,
                           boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().total_size()),
                           [this](const boost::system::error_code& ec, std::size_t s) {
                             if(!ec) {
                               write_msgs_.pop_front();

                               if(!write_msgs_.empty())
                                 do_write();

                             } else {
                               socket_.close();
                             }
                           });
}












