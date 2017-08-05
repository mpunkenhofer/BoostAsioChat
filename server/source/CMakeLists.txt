include_directories(../include ../../common/include)

add_library(ChatServer chat_channel.cpp chat_server.cpp chat_user.cpp chat_user_manager.cpp)

add_executable(Server server.cpp)
target_link_libraries(ChatServer ChatCommon)
target_link_libraries(Server pthread boost_system ChatServer)