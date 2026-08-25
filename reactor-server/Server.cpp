//
// Created by yuzhang on 2026/8/10.
//
#include <complex>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>

#include "include/Channel.h"
#include "include/Epoll.h"
#include "include/InetAddress.h"
#include "include/Socket.h"

int main(int argc, char* argv[])
{
    // 创建服务端用于监听的server_socket_fd
    Socket serv_socket(createNonBlocking());
    // 设置fd的属性
    serv_socket.setReuseAddr(true);
    serv_socket.setReusePort(true);
    serv_socket.setTcpNoDelay(true);
    serv_socket.setKeepAlive(true);

    InetAddress serv_addr(argv[1], atoi(argv[2]));

    serv_socket.bind(serv_addr);
    serv_socket.listen();

    std::cout << "[Server] Listening on " << argv[1] << ":" << argv[2] << std::endl;

    Epoll epoll;
    auto* serv_channel = new Channel(&epoll, serv_socket.fd(), true);
    serv_channel->enableReading();

    std::cout << "[Server] Server is running, waiting for connections..." << std::endl;

    while (true)
    {
        std::vector<Channel*> ready_channels = epoll.loop();
        // 【核心区别】与其他的核心区别，只遍历就绪的事件
        for (auto channel : ready_channels)
        {

        }
    }
}
