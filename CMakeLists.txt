cmake_minimum_required(VERSION 3.5)
project(BoostAsioChat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic")

#Source directories
add_subdirectory(client)
add_subdirectory(common)
add_subdirectory(server)