//
// Created by necator on 8/8/17.
//

#ifndef BOOSTASIOCHAT_CHAT_MESSAGE_H
#define BOOSTASIOCHAT_CHAT_MESSAGE_H


#include <string>
#include <array>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/asio/buffer.hpp>

#include "chat_message_type.h"

class chat_message {
public:
    chat_message();
    chat_message(const std::string& source,
                 const std::string& target,
                 const std::string& content,
                 chat_message_type t = chat_message_type::text);

    static constexpr std::size_t header_length = 10;
    static constexpr std::size_t content_max_length = 512;
    static constexpr std::size_t target_max_length = 20;
    static constexpr std::size_t source_max_length = target_max_length;

    chat_message_type type() const;
    std::string target() const;
    std::string source() const;
    std::string content() const;

    void set_target(std::string t);
    void set_source(std::string s);
    void set_content(std::string c);

    std::vector<boost::asio::const_buffer> generate_buffers();
private:
    std::string source_;
    std::string target_;
    std::string content_;
    chat_message_type type_;

    std::vector<boost::asio::const_buffer> write_buffers_;
    bool buffers_cached_;

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version __attribute__ ((unused)))
    {
        ar & type_;
        ar & source_;
        ar & target_;
        ar & content_;
    }
};

std::ostream &operator<<(std::ostream &os, const chat_message &msg);
std::string serialize_chat_message(const chat_message& msg);
chat_message deserialize_chat_message(const std::vector<char>& data);

#endif //BOOSTASIOCHAT_CHAT_MESSAGE_H
