//
// Created by necator on 8/1/17.
//

#include "message.h"

#include <sstream>
#include <iomanip>
#include <cstring>

message::message() {

}

message::message(const std::string &s, message_type type) {
    if (s.size() < max_length) {
        std::copy(s.begin(), s.end(), content_begin());
        head_.type = type;
        head_.size = s.size();

        std::memcpy(target_begin(), "-\0", 2);

        encode();
    } else
        throw std::length_error("s > max_length(" + std::to_string(max_length) + ")");
}

message::message(const std::string &s, const std::string &target) : message(s) {
    if (target.size() < target_size) {
        std::copy(target.begin(), target.end(), target_begin());
    } else
        throw std::length_error("target > max_length(" + std::to_string(target_size) + ")");
}

message::message_type message::type() const {
    return head_.type;
}

std::string message::content() const {
    return std::string(content_begin(), content_end());
}

std::string message::target() const {
    return std::string(target_begin(), target_end());
}

char *message::data() {
    return buffer.begin();
}

const char *message::data() const {
    return buffer.begin();
}

char *message::head_begin() {
    return buffer.begin();
}

const char *message::head_begin() const {
    return buffer.begin();
}

char *message::head_end() {
    return buffer.begin() + header_size();
}

const char *message::head_end() const {
    return buffer.begin() + header_size();
}

char *message::content_begin() {
    return buffer.begin() + header_size() + target_size;
}

const char *message::content_begin() const {
    return buffer.begin() + header_size() + target_size;
}

char *message::content_end() {
    return buffer.begin() + header_size() + target_size + head_.size;
}

const char *message::content_end() const {
    return buffer.begin() + header_size() + target_size + head_.size;
}

char *message::target_begin() {
    return buffer.begin() + header_size();
}

const char *message::target_begin() const {
    return buffer.begin() + header_size();
}

char *message::target_end() {
    return target_begin() + std::strlen(target_begin()) + 1;
}

const char *message::target_end() const {
    return target_begin() + std::strlen(target_begin()) + 1;
}

std::size_t message::content_size() const {
    return head_.size;
}

std::size_t message::total_size() const {
    return head_.size + header_size() + target_size;
}

void message::encode() {
    std::stringstream ss;

    ss << std::setw(message_type_length_) << message_type_to_int(head_.type)
       << std::setw(message_size_length_) << head_.size;

    std::string s = ss.str();

    if (s.size() != header_size())
        throw std::runtime_error("message encoding error!");

    std::copy(s.begin(), s.end(), head_begin());
}

bool message::decode() {
    std::string s_type(head_begin(), head_begin() + message_type_length_);
    std::string s_size(head_begin() + message_type_length_,
                       head_begin() + message_type_length_ + message_size_length_);

    std::stringstream ss_type(s_type);
    std::stringstream ss_size(s_size);

    int mt;

    if (!(ss_type >> mt) || !(ss_size >> head_.size))
        return false;

    head_.type = int_to_message_type(mt);

    return true;
}

int message::message_type_to_int(message::message_type mt) {
    switch (mt) {
        case message_type::command:
            return 0;
        case message_type::text:
            return 1;
        default:
            return -1;
    }
}

std::string message::message_type_to_string(message::message_type mt) {
    static std::vector<std::string> message_types{"command", "text"};

    return message_types[message_type_to_int(mt)];
}

message::message_type message::int_to_message_type(int i) {
    switch (i) {
        case 0:
            return message_type::command;
        case 1:
            return message_type::text;
        default:
            return message_type::unknown;
    }
}

std::string message::debug_string() const {
    std::string type_s = message::message_type_to_string(type());
    std::string content_s = content();
    std::string target_s = target();

    std::stringstream ss;

    ss << "Type: " << type_s << " | Target: " << target_s << " | Content: " << content_s;

    return ss.str();
}

std::ostream &operator<<(std::ostream &os, const message &msg) {
    if (os) {
        std::string type_s = message::message_type_to_string(msg.type());
        std::string content_s = msg.content();
        std::string target_s = msg.target();

        os << "Type: " << type_s << " | Target: " << target_s << " | Content: " << content_s;
    }

    return os;
}