include_directories(${CMAKE_SOURCE_DIR}/common/include)

add_library(ChatCommon
        message.cpp         ${CMAKE_SOURCE_DIR}/common/include/message.h
        easylogging++.cc    ${CMAKE_SOURCE_DIR}/common/include/easylogging++.h)