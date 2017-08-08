cmake_minimum_required(VERSION 3.2)
project(BoostAsioChat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wstrict-aliasing -pedantic -Wunreachable-code")

add_definitions(-DELPP_THREAD_SAFE)

#Source directories
add_subdirectory(client)
add_subdirectory(common)
add_subdirectory(server)