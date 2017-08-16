include_directories(${CMAKE_SOURCE_DIR}/common/include)

add_library(ChatCommon
        chat_message.cpp        ${CMAKE_SOURCE_DIR}/common/include/chat_message.h
        chat_message_type.cpp   ${CMAKE_SOURCE_DIR}/common/include/chat_message_type.h
        # chat_session.cpp        ${CMAKE_SOURCE_DIR}/common/include/chat_session.h
        easylogging.cc ${CMAKE_SOURCE_DIR}/common/include/easylogging.h)

target_link_libraries(ChatCommon boost_serialization)