include_directories(../include ../../common/include)

add_library(ChatClient chat_client.cpp)

add_executable(Client client.cpp)
target_link_libraries(ChatClient ChatCommon)
target_link_libraries(Client pthread boost_system ChatClient)