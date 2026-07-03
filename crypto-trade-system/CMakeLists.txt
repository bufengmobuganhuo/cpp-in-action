cmake_minimum_required(VERSION 4.2)
project(crypto_trade_system)

set(CMAKE_CXX_STANDARD 17)

add_executable(crypto_trade_system src/main.cpp
        include/MerkelMain.h
        include/Wallet.h
        src/Wallet.cpp
        include/OrderBookEntry.h
        include/CSVReader.h
        include/OrderBook.h
        src/MerkelMain.cpp
        src/CSVReader.cpp
)
