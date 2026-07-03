cmake_minimum_required(VERSION 4.2)
project(supermarket_billing_system)

set(CMAKE_CXX_STANDARD 17)

add_executable(supermarket_billing_system src/main.cpp
        src/billing.cpp
        include/billing.h)
