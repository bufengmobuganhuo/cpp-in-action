set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

add_executable(epoll_demo Main.cpp Server.cpp Client.cpp)
target_link_libraries(epoll_demo pthread)