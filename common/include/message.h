//
// Created by necator on 8/1/17.
//

#ifndef BOOSTCHAT_MESSAGE_H
#define BOOSTCHAT_MESSAGE_H

#include <array>
#include <string>
#include <boost/system/error_code.hpp>
#include <vector>

class message {
public:
    enum class message_type {
        text,
        command,
        status,
        unknown
    };

    static int message_type_to_int(message::message_type mt);

    static message::message_type int_to_message_type(int i);

    static std::string message_type_to_string(message::message_type mt);

    message();

    message(const std::string &s, message::message_type type = message::message_type::text);

    message(const std::string &s, const std::string &target);

    static constexpr std::size_t header_size() {
        return message_type_length_ + message_size_length_;
    }

    static const std::size_t max_length = 512;
    static const std::size_t target_size = 20;

    message_type type() const;

    std::string content() const;

    std::string target() const;

    std::size_t content_size() const;

    std::size_t total_size() const;

    char *data();

    const char *data() const;

    char *head_begin();

    char *head_end();

    const char *head_begin() const;

    const char *head_end() const;

    char *content_begin();

    char *content_end();

    const char *content_begin() const;

    const char *content_end() const;

    char *target_begin();

    char *target_end();

    const char *target_begin() const;

    const char *target_end() const;

    bool decode();

    void encode();

    std::string debug_string() const;
private:
    static constexpr std::size_t message_type_length_ = 4;
    static constexpr std::size_t message_size_length_ = 4;

    struct head {
        message_type type;
        std::size_t size;

        head() : type(message_type::unknown), size(0) {}
    };

    head head_;

    std::array<char, max_length + message_type_length_ + message_size_length_ + target_size + 1> buffer;
};

std::ostream &operator<<(std::ostream &os, const message &msg);

#endif //BOOSTCHAT_MESSAGE_H
