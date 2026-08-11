set(CMAKE_CXX_STANDARD 11)

add_executable(select_demo Main.cpp Server.cpp Client.cpp)
target_link_libraries(select_demo pthread)