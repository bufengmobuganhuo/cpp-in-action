//
// Created by yuzhang on 2026/8/10.
//
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <vector>

constexpr int kServerPort = 8080;
constexpr int kBufferSize = 1024;
// epoll_wait每次最多返回的就绪事件数
constexpr int kMaxEvents = 1024;

void runServer()
{
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd == -1)
    {
        perror("[EpollServer] socket creation failed");
        return;
    }

    int reuse_addr_opt = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_opt, sizeof(reuse_addr_opt)) < 0)
    {
        perror("[EpollServer] setsockopt failed");
        return;
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(kServerPort);

    if (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("[EpollServer] bind failed");
        close(server_socket_fd);
        return;
    }

    if (listen(server_socket_fd, 5) < 0)
    {
        perror("[EpollServer] listen failed");
        close(server_socket_fd);
        return;
    }

    std::cout << "[EpollServer] Listening on port " << kServerPort << std::endl;

    // 创建epoll实例，返回一个专属的epoll id
    int epoll_fd = epoll_create(1);
    if (epoll_fd == -1)
    {
        perror("[EpollServer] epoll_create failed");
        close(server_socket_fd);
        return;
    }

    // 将服务端socket注册到epoll的红黑树中
    struct epoll_event server_event{};
    server_event.events = EPOLLIN;
    server_event.data.fd = server_socket_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket_fd, &server_event) < 0)
    {
        perror("[EpollServer] epoll_ctl ADD server failed");
        close(server_socket_fd);
        close(epoll_fd);
        return;
    }

    // 用于接收epoll_wait返回的就绪事件
    std::vector<struct epoll_event> ready_events(kMaxEvents);

    while (true)
    {
        // 阻塞等待事件发生
        // 参数2：事件数组指针
        // 参数3：数组最大容量
        // 参数4：超时时间，-1表示无限阻塞
        int ready_count = epoll_wait(epoll_fd, ready_events.data(), kMaxEvents, -1);

        if (ready_count < 0 && errno != EINTR)
        {
            perror("[EpollServer] epoll_wait failed");
            break;
        }

        // 【核心区别】与其他的核心区别，只遍历就绪的事件
        for (int i = 0; i < ready_count; ++i)
        {
            int current_fd = ready_events[i].data.fd;
            // 1. 处理新连接
            if (current_fd == server_socket_fd)
            {
                struct sockaddr_in client_addr{};
                socklen_t addr_len = sizeof(client_addr);
                int new_socket = accept(server_socket_fd, (struct sockaddr*)&client_addr, &addr_len);
                if (new_socket >= 0)
                {
                    std::cout << "[EpollServer] New connection, fd: " << new_socket << std::endl;

                    struct epoll_event client_event{};
                    client_event.events = EPOLLIN;
                    client_event.data.fd = new_socket;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &client_event);
                }
            }
            else
            {
                // 处理客户端数据
                char buffer[kBufferSize] = {0};
                int bytes_read = read(current_fd, buffer, kBufferSize - 1);

                if (bytes_read <= 0)
                {
                    std::cout << "[EpollServer] Client disconnected, fd: " << current_fd << std::endl;

                    close(current_fd);
                    // 【核心区别】从epoll红黑树中移除该fd
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, nullptr);
                }
                else
                {
                    buffer[bytes_read] = '\0';
                    std::cout << "[EpollServer] Received: " << buffer << std::endl;
                    send(current_fd, buffer, bytes_read, 0);
                }
            }
        }
    }

    close(server_socket_fd);
    close(epoll_fd);
}
