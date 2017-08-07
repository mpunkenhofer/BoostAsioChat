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









