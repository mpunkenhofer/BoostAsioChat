include_directories(${CMAKE_SOURCE_DIR}/client/include ${CMAKE_SOURCE_DIR}/common/include)

add_library(ChatClient
        chat_client.cpp ${CMAKE_SOURCE_DIR}/client/include/chat_client.h)

add_executable(Client
        client.cpp)

target_link_libraries(ChatClient
        ChatCommon)

target_link_libraries(Client
        pthread
        boost_system
        boost_thread
        ChatClient)