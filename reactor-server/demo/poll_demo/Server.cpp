//
// Created by yuzhang on 2026/8/10.
//
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <vector>


// 【规范1】使用 constexpr 替代宏定义
constexpr int kServerPort = 8080;
constexpr int kBufferSize = 1024;

void runServer()
{
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd == -1)
    {
        perror("[Server] socket creation failed");
        return;
    }

    int reuse_addr_opt = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_opt, sizeof(reuse_addr_opt)) < 0)
    {
        perror("[Server] setsockopt failed");
        return;
    }

    struct sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(kServerPort);

    if (bind(server_socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
        perror("[Server] listen failed");
        close(server_socket_fd);
        return;
    }

    if (listen(server_socket_fd, 5) < 0)
    {
        perror("[Server] listen failed");
        close(server_socket_fd);
        return;
    }
    std::cout << "[Server] Listening on port " << kServerPort << std::endl;

    // 使用vector存储pollfd结构体，动态管理，无数量上限
    std::vector<struct pollfd> poll_fds;

    // 将服务端监听 socket 加入监控列表
    struct pollfd server_poll_fd{};
    server_poll_fd.fd = server_socket_fd;
    server_poll_fd.events = POLLIN;
    poll_fds.push_back(server_poll_fd);

    while (true)
    {
        // -1表示没有超时时间
        int activity = poll(poll_fds.data(), poll_fds.size(), -1);

        if (activity < 0 && errno != EINTR)
        {
            perror("[Server] poll failed");
            break;
        }

        // 线性遍历整个数组，时间复杂度O(n)，
        for (size_t i = 0; i < poll_fds.size(); ++i)
        {
            // 检查revents字段是否包含POLLIN事件
            if (poll_fds[i].revents & POLLIN)
            {
                // 如果是服务端socket，说明有新连接
                if (poll_fds[i].fd == server_socket_fd)
                {
                    struct sockaddr_in client_addr{};
                    socklen_t addr_len = sizeof(client_addr);
                    int new_socket = accept(server_socket_fd, (struct sockaddr*)&client_addr, &addr_len);
                    if (new_socket >= 0)
                    {
                        std::cout << "[Server] New connection, fd: " << new_socket << std::endl;

                        // 将新连接放入监控列表
                        struct pollfd client_poll_fd{};
                        client_poll_fd.fd = new_socket;
                        client_poll_fd.events = POLLIN;
                        poll_fds.push_back(client_poll_fd);
                    }
                }
                else
                {
                    // 如果是客户端socket，说明有数据可读
                    char buffer[kBufferSize] = {0};
                    int bytes_read = read(poll_fds[i].fd, buffer, kBufferSize - 1);
                    if (bytes_read <= 0)
                    {
                        std::cout << "[Server] Client disconnected, fd: " << std::endl;

                        close(poll_fds[i].fd);
                        // 从vector中移除
                        poll_fds.erase(poll_fds.begin() + i);
                        // 删除元素后，索引回退，防止漏掉下一个元素
                        --i;
                    }
                    else
                    {
                        buffer[bytes_read] = '\0';
                        std::cout << "[Server] Received: " << buffer << std::endl;
                        send(poll_fds[i].fd, buffer, bytes_read, 0);
                    }
                }
            }
        }
    }
    close(server_socket_fd);
}
