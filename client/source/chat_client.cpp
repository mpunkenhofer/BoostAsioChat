//
// Created by necator on 8/4/17.
//

#include "chat_client.h"

//#define ELPP_DISABLE_INFO_LOGS
#include "easylogging++.h"

chat_client::chat_client(boost::asio::io_service &io_service,
                         boost::asio::ip::tcp::resolver::iterator endpoint_iterator) :
        io_service_(io_service),
        socket_(io_service),
        write_strand_(io_service) {
    do_connect(endpoint_iterator);
}

void chat_client::do_connect(boost::asio::ip::tcp::resolver::iterator endpoint) {
    LOG(INFO) << "connect to server...";

    boost::asio::async_connect(socket_,
                               endpoint,
                               [this](const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator) {
                                   if (!ec)
                                       do_read_header();
                               });
}

void chat_client::print(const chat_message &msg) {
    std::cout << msg.target() << "::" << msg.source() << ": " << msg.content() << '\n';
}

void chat_client::error_handler() {
    LOG(ERROR) << "an error occurred, closing connection.";
    close();
}

void chat_client::close() {
    io_service_.post([this](){
        if(socket_.is_open()) {
            socket_.shutdown(socket_.shutdown_both);
            socket_.close();
        }
    });
}

void chat_client::write(chat_message &msg) {
    io_service_.post(write_strand_.wrap([this, msg]() {
        auto write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(std::move(msg));

        if (!write_in_progress)
            do_write();
    }));
}

void chat_client::do_read_header() {
    LOG(INFO) << "waiting for a msg...";

    boost::asio::async_read(socket_,
                            boost::asio::buffer(inbound_header_, inbound_header_.size()),
                            [this](const boost::system::error_code &ec,
                                         std::size_t s __attribute__ ((unused))) {
                                if (!ec) {
                                    std::istringstream is(std::string(inbound_header_.begin(), inbound_header_.end()));
                                    std::size_t inbound_data_size = 0;

                                    if (!(is >> std::hex >> inbound_data_size))
                                        error_handler();

                                    inbound_data_.resize(inbound_data_size);

                                    do_read_message();
                                } 
                                else if(ec == boost::asio::error::operation_aborted)
                                    LOG(INFO) << "operation aborted.";
                                else {
                                    LOG(ERROR) << "Error: " << ec.message();
                                    error_handler();
                                }
                            });
}

void chat_client::do_read_message() {
    boost::asio::async_read(socket_,
                            boost::asio::buffer(inbound_data_, inbound_data_.size()),
                            [this](const boost::system::error_code &ec,
                                         std::size_t s __attribute__ ((unused))) {
                                if (!ec) {
                                    print(deserialize_chat_message(inbound_data_));

                                    do_read_header();
                                } 
                                else if(ec == boost::asio::error::operation_aborted)
                                    LOG(INFO) << "operation aborted.";
                                else {
                                    LOG(ERROR) << "Error: " << ec.message();
                                    error_handler();
                                }
                            });
}

void chat_client::do_write() {
    boost::asio::async_write(socket_,
                             write_msgs_.front().generate_buffers(),
                             write_strand_.wrap([this](const boost::system::error_code &ec,
                                                             std::size_t s __attribute__ ((unused))) {
                                 if (!ec) {
                                     write_msgs_.pop_front();

                                     if (!write_msgs_.empty())
                                         do_write();

                                 } else {
                                     LOG(ERROR) << "Error: " << ec.message();
                                     error_handler();
                                 }
                             }));
}