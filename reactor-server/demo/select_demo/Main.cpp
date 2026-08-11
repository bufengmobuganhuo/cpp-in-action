//
// Created by yuzhang on 2026/8/10.
//
#include <thread>
#include <iostream>

// 申明外部函数
extern void runServer();
extern void runClient();

int main()
{
    std::cout << "=== Starting Server and Client in one process ===" << std::endl;

    std::thread server_thread(runServer);
    std::thread client_thread(runClient);

    server_thread.join();
    client_thread.join();
}