//
// Created by yuzhang on 2026/8/10.
//
#include <iostream>
#include <cstring>
#include <emmintrin.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <vector>

#define PORT 8080
#define BUFFER_SIZE 1024

void runServer()
{
    // 1. 创建套接字，IPV4，流式传输，默认协议
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 允许端口重用，防止服务端重启时因端口处于 TIME_WAIT 状态而绑定失败
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. 配置服务端地址信息
    struct sockaddr_in address{};

    // IPv4
    address.sin_family = AF_INET;
    // 绑定本机所有可用IP
    address.sin_addr.s_addr = INADDR_ANY;
    // 将端口号转换为网络字节序
    address.sin_port = htons(PORT);

    // 3. 绑定IP和端口，并开启监听
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);
    std::cout << "[Server] Listening on port " << PORT << std::endl;

    // 4. 维护一个已连接的客户端 fd 列表
    std::vector<int> client_fds;
    // 记录当前监控的最大fd
    int max_fd = server_fd;

    while (true)
    {
        // 每次都需要重新初始化fd_set位图，因为select返回时会修改该位图，仅保留就绪的fd
        fd_set readfds;
        FD_ZERO(&readfds);

        // 将监听socket 加入监控集合，用于接收新链接
        FD_SET(server_fd, &readfds);

        for (int fd : client_fds)
        {
            FD_SET(fd, &readfds);
            if (fd > max_fd)
            {
                max_fd = fd;
            }
        }

        // 调用select，阻塞等待事件发生
        // 参数1：max_fd + 1（内核遍历位图的边界）
        // 参数2：读事件集合
        // 参数3，4：写事件和异常事件集合（此处不需要）
        // 参数5：超时时间
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        // 如果select出错且不是因为被信号中断，则退出循环
        if (activity < 0 && errno != EINTR)
        {
            break;
        }

        // 5. 检查是否有新的客户端连接请求
        if (FD_ISSET(server_fd, &readfds))
        {
            struct sockaddr_in client_addr{};
            socklen_t addrlen = sizeof(client_addr);
            int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);

            if (new_socket >= 0)
            {
                std::cout << "[Server] New connnection, fd: " << new_socket << std::endl;
                // 将新连接的fd加入管理列表
                client_fds.push_back(new_socket);
            }
        }

        // 6. 遍历所有客户端，检查是否有数据可读
        for (auto it = client_fds.begin(); it != client_fds.end();)
        {
            int fd = *it;
            // 使用FD_ISSET宏检查fd是否在select返回的就绪集合中
            if (FD_ISSET(fd, &readfds))
            {
                char buffer[BUFFER_SIZE] = {0};
                int bytes_read = read(fd, buffer, BUFFER_SIZE);

                // 返回值<=0表示客户端断开连接或发生错误
                if (bytes_read <= 0)
                {
                    std::cout << "[Server] Client disconnected, fd " << fd << std::endl;
                    close(fd);
                    // 从vector中移除已经断开的fd, erase返回下一个有效迭代器
                    it = client_fds.erase(it);
                }
                else
                {
                    // 收到数据，打印并原样返回给客户端
                    std::cout << "[Server] Received: " << buffer;
                    send(fd, buffer, bytes_read, 0);
                    // 正常处理完，迭代器后移
                    ++it;
                }
            }
            else
            {
                // 该fd没有就绪，读下一个
                ++it;
            }
        }
    }
    close(server_fd);
}