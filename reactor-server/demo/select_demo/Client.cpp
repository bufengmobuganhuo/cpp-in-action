//
// Created by yuzhang on 2026/8/10.
//
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void runClient()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // 配置服务端地址
    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // 等待1秒，确保服务端已经启动并开始listen
    sleep(5);

    // 发起TCP链接
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return;
    }
    std::cout << "[Client] Connected to server." << std::endl;

    char buffer[BUFFER_SIZE] = {0};
    while (true)
    {
        // 从标准输入获取用户消息
        std::cout << "[Client] Enter message: ";
        std::cin.getline(buffer, BUFFER_SIZE);
        send(sock, buffer, strlen(buffer), 0);

        // 使用select实现带超时的非阻塞读取
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        // 设置2秒超时
        struct timeval timeout = {2, 0};

        // 等待服务器回复，socket + 1 是因为服务端只有一个socket
        int activity = select(sock + 1, &readfds, NULL, NULL, &timeout);

        if (activity > 0 && FD_ISSET(sock, &readfds))
        {
            // 清空buffer
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_read = read(sock, buffer, BUFFER_SIZE);
            if (bytes_read > 0)
            {
                std::cout << "[Client] Server replay: " << buffer << std::endl;
            }
        }
        else if (activity == 0)
        {
            std::cout << "[Client] Timeout!" << std::endl;
        }
        else
        {
            // 发生错误
            break;
        }
    }
    close(sock);
}