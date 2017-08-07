//
// Created by necator on 8/1/17.
//

#include "chat_user_manager.h"

chat_user_manager::chat_user_manager()
{

}

void chat_user_manager::start(chat_user_ptr c)
{
    connections_.insert(c);
    c->start();
}

void chat_user_manager::stop(chat_user_ptr c)
{
  connections_.erase(c);
  c->stop();
}

void chat_user_manager::stop_all()
{
  for(auto c : connections_)
    c->stop();

  connections_.clear();
}

chat_user_ptr chat_user_manager::user(const std::string& name) {
    auto it = std::find_if(connections_.begin(), connections_.end(), [&name](chat_user_ptr usr) {
        return usr->name() == name;
    });

    if(it != connections_.end())
        return *it;
    else
        return nullptr;
}

bool chat_user_manager::user_exists(const std::string& name) const {
    auto it = std::find_if(connections_.begin(), connections_.end(), [&name](chat_user_ptr usr){
        return usr->name() == name;
    });

    return (it != connections_.end()) ? true : false;
}






