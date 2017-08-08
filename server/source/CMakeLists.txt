include_directories(${CMAKE_SOURCE_DIR}/server/include ${CMAKE_SOURCE_DIR}/common/include)
link_directories(${CMAKE_SOURCE_DIR}/server/include ${CMAKE_SOURCE_DIR}/common/include)

add_library(ChatServer
        chat_channel.cpp        ${CMAKE_SOURCE_DIR}/server/include/chat_channel.h
        chat_server.cpp         ${CMAKE_SOURCE_DIR}/server/include/chat_server.h
        chat_user.cpp           ${CMAKE_SOURCE_DIR}/server/include/chat_user.h
        chat_user_manager.cpp   ${CMAKE_SOURCE_DIR}/server/include/chat_user_manager.h)

add_executable(Server
        server.cpp)

target_link_libraries(ChatServer
        ChatCommon)

target_link_libraries(Server
        boost_system
        pthread
        boost_thread
        ChatServer)