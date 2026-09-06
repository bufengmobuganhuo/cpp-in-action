//
// Created by yuzhang on 2026/8/11.
//
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "include/InetAddress.h"

constexpr int k_buffer_size = 1024;

int main(int argc, char* argv[])
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // 配置服务端地址
    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    // 等待1秒，确保服务端已经启动并开始listen
    sleep(1);

    // 发起TCP链接
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return -1;
    }
    std::cout << "[Client] Connected to server." << std::endl;

    char buffer[k_buffer_size] = {0};
    for (int i = 0; i < 1; i++)
    {
        memset(buffer, 0, sizeof buffer); // 把buffer内容置空
        sprintf(buffer, "这是第%d个超级女声", i);

        char tmp_buffer[1024]; // 消息长度 + 报文内容
        memset(tmp_buffer, 0, sizeof tmp_buffer);
        int len = strlen(buffer); // 记录消息的长度
        memcpy(tmp_buffer, &len, 4); // 消息长度
        memcpy(tmp_buffer + 4, buffer, len); // 报文内容

        send(sock, tmp_buffer, len + 4, 0);
    }

    for (int i = 0; i < 1; i++)
    {
        int len;
        recv(sock, &len, 4, 0); // 读取报文长度

        memset(buffer, 0, sizeof buffer);
        recv(sock, buffer, len, 0); // 读取报文内容
        printf("recv: %s\n", buffer);
    }
    close(sock);
    sleep(100);
}
