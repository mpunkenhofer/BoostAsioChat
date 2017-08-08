//
// Created by necator on 8/8/17.
//

#include <sstream>
#include "chat_message.h"

chat_message::chat_message() : type_(chat_message_type::unknown), buffers_cached_(false) {

}

chat_message::chat_message(const std::string &source, const std::string &target, const std::string &content,
                           chat_message_type t) : type_(t), buffers_cached_(false) {
    if(source.size() > source_max_length || target.size() > target_max_length || content.size() > content_max_length)
        throw std::runtime_error("chat message: content, source or target over character limit!");

    source_ = source;
    target_ = target;
    content_ = content;
}

chat_message_type chat_message::type() const {
    return type_;
}

std::string chat_message::target() const {
    return target_;
}

std::string chat_message::source() const {
    return source_;
}

std::string chat_message::content() const {
    return content_;
}

void chat_message::set_target(std::string t) {
    if(t.size() > target_max_length)
        throw std::runtime_error("chat_message::set_target(t): target max len: " + std::to_string(target_max_length));

    buffers_cached_ = false;
    target_ = std::move(t);
}

void chat_message::set_source(std::string s) {
    if(s.size() > source_max_length)
        throw std::runtime_error("chat_message::set_source(s): source max len: " + std::to_string(source_max_length));

    buffers_cached_ = false;
    source_ = std::move(s);
}

void chat_message::set_content(std::string c) {
    if(c.size() > content_max_length)
        throw std::runtime_error("chat_message::set_content(c): content max len: " + std::to_string(content_max_length));

    buffers_cached_ = false;
    content_ = std::move(c);
}

std::vector<boost::asio::const_buffer> chat_message::generate_buffers() {
    static std::string header;
    static std::string data;

    if(!buffers_cached_) {
        write_buffers_.clear();

        data = serialize_chat_message(*this);

        std::ostringstream os;

        os << std::setw(header_length) << std::hex << data.size();

        if (!os || os.str().size() != header_length)
            return std::vector<boost::asio::const_buffer>();

        header = os.str();

        write_buffers_.push_back(boost::asio::buffer(header));
        write_buffers_.push_back(boost::asio::buffer(data));

        buffers_cached_ = true;
    }

    return write_buffers_;
}

std::ostream &operator<<(std::ostream &os, const chat_message &msg) {
    if (os) {
        os << "Type: " << to_string(msg.type())
           << " | Source: " << msg.source()
           << " | Target: " << msg.target()
           << " | Content: " << msg.content();
    }

    return os;
}

std::string serialize_chat_message(const chat_message& msg) {
    std::ostringstream os;

    boost::archive::text_oarchive archive(os);

    archive << msg;

    return os.str();
}

chat_message deserialize_chat_message(const std::vector<char>& data) {
    std::string s(data.begin(), data.end());

    std::istringstream is(s);

    boost::archive::text_iarchive archive(is);

    chat_message msg;
    archive >> msg;

    return msg;
}

