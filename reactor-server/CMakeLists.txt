cmake_minimum_required(VERSION 3.18)
project(reactor_server)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

add_subdirectory(demo/select_demo)
add_subdirectory(demo/poll_demo)
add_subdirectory(demo/epoll_demo)
add_executable(netserver Server.cpp)
add_executable(netclient Client.cpp)
