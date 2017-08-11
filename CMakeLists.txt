cmake_minimum_required(VERSION 3.2)
project(BoostAsioChat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wstrict-aliasing -pedantic -Wunreachable-code")

#Source directories
add_subdirectory(client)
add_subdirectory(common)
add_subdirectory(server)

#Logging
add_definitions(-DELPP_THREAD_SAFE)
add_definitions(-DELPP_DISABLE_INFO_LOGS)
add_definitions(-DELPP_DISABLE_WARNING_LOGS)
#add_definitions(-DELPP_DISABLE_ERRORLOGS)

set_target_properties(Server ChatServer
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/lib/server"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/lib/server"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/bin/server"
        )

set_target_properties(Client ChatClient
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/lib/client"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/lib/client"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/bin/client"
        )

set_target_properties(ChatCommon
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/lib/common"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/lib/common"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/bin/common"
        )