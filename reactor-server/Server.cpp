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
#include <netinet/tcp.h>      // TCP_NODELAY需要包含这个头文件。

// epoll_wait每次最多返回的就绪事件数
constexpr int kMaxEvents = 1024;
constexpr int kBufferSize = 1024;

int main(int argc, char* argv[])
{
    // 创建服务端用于监听的server_socket_fd
    int server_socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (server_socket_fd < 0)
    {
        perror("[EpollServer] socket creation failed");
        return -1;
    }

    // 设置fd的属性
    int opt = 1;
    // 允许重用本地地址，防止服务器重启时因端口被占用而绑定失败 (Address already in use)
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
    // 禁用 Nagle 算法，关闭延迟确认，使数据立即发送以降低网络延迟 (注: 此处 level 建议为 IPPROTO_TCP)
    setsockopt(server_socket_fd, SOL_SOCKET, TCP_NODELAY, &opt, sizeof opt);
    // 允许重用本地端口，允许多个进程或线程绑定并监听同一个端口，常用于多进程负载均衡
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof opt);
    // 开启 TCP 保活机制 (Keep-Alive)，定期发送探测包以检测连接是否仍然存活
    setsockopt(server_socket_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof opt);

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    // 服务端监听的IP地址
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    // 服务端监听的端口
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (bind(server_socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("[Server] bind failed");
        close(server_socket_fd);
        return -1;
    }


    // 将 socket 设置为监听状态，等待客户端连接
    // 第二个参数 backlog 指定内核中该 socket 对应挂起连接队列的最大长度
    // 128 是常见的经验值，表示允许最多 128 个已完成三次握手但尚未被 accept() 取走的连接排队等待
    // 注意：实际值可能会被内核调整（如 Linux 中会自动向上取整到下一个 2 的幂），且受系统参数 somaxconn 限制
    if (listen(server_socket_fd, 128) < 0)
    {
        perror("[Server] listen failed");
        return -1;
    }

    std::cout << "[Server] Listening on " << argv[1] << ":" << argv[2] << std::endl;


    // 创建一个 epoll 实例，返回一个指向该实例的文件描述符 epoll_fd
    // 参数 1 表示期望监听的文件描述符数量（size 提示值）
    // 注意：自 Linux 2.6.8 起，该参数已被内核忽略，但必须大于 0，因此通常传入 1 即可
    int epoll_fd = epoll_create(1);
    if (epoll_fd < 0)
    {
        perror("[Server] epoll_create failed");
        close(server_socket_fd);
        return -1;
    }

    // 为服务端的server_socket_fd准备读事件
    struct epoll_event server_event{};
    server_event.data.fd = server_socket_fd;
    // 让epoll监视listenfd的读事件，采用水平触发。
    server_event.events = EPOLLIN;

    // 使用 epoll_ctl 将服务端的监听 socket 注册到 epoll 实例中
    // 参数 1: epoll_fd - epoll 实例的文件描述符，由 epoll_create 创建
    // 参数 2: EPOLL_CTL_ADD - 控制命令，表示向 epoll 实例中添加一个新的文件描述符（其他命令有 EPOLL_CTL_MOD 修改, EPOLL_CTL_DEL 删除）
    // 参数 3: server_socket_fd - 需要被监听的文件描述符，即服务端的 listen fd
    // 参数 4: &server_event - 指向 epoll_event 结构体的指针，指定了要监听的事件类型（如 EPOLLIN）及关联的用户数据（如 data.fd）
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket_fd, &server_event) < 0)
    {
        perror("[EpollServer] epoll_ctl ADD server failed");
        close(server_socket_fd);
        close(epoll_fd);
        return -1;
    }

    std::vector<epoll_event> ready_events(kMaxEvents);
    std::cout << "[Server] Server is running, waiting for connections..." << std::endl;
    while (true)
    {
        // 阻塞等待事件发生
        // 参数2：事件数组指针
        // 参数3：数组最大容量
        // 参数4：超时时间，-1表示无限阻塞
        int ready_count = epoll_wait(epoll_fd, ready_events.data(), kMaxEvents, -1);

        if (ready_count < 0 && errno != EINTR)
        {
            perror("[Server] epoll_wait failed");
            break;
        }

        // 【核心区别】与其他的核心区别，只遍历就绪的事件
        for (int i = 0; i < ready_count; ++i)
        {
            struct epoll_event current_event = ready_events[i];
            int current_fd = current_event.data.fd;

            if (current_event.events & EPOLLRDHUP)
            {
                // 对方已关闭
                std::cout << "[Server] event_fd=" << current_fd << "is closed" << std::endl;
                close(current_fd);
            }
            // 处理新连接
            else if (current_fd == server_socket_fd)
            {
                struct sockaddr_in client_addr{};
                socklen_t addr_len = sizeof(client_addr);
                int new_socket = accept4(server_socket_fd, (struct sockaddr*)&client_addr, &addr_len, SOCK_NONBLOCK);
                if (new_socket >= 0)
                {
                    std::cout << "[EpollServer] New connection, fd: " << new_socket << std::endl;

                    struct epoll_event client_event{};
                    client_event.events = EPOLLIN;
                    client_event.data.fd = new_socket;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &client_event);
                }
            }
            else if (current_event.events & (EPOLLIN | EPOLLPRI))
            {
                // 客户端有数据可读，要一直读完为止
                // 处理客户端数据
                char buffer[kBufferSize] = {0};
                while (true)
                {
                    // 清理buffer
                    bzero(&buffer, sizeof(buffer));
                    int bytes_read = read(current_fd, buffer, kBufferSize - 1);
                    if (bytes_read > 0)
                    {
                        buffer[bytes_read] = '\0';
                        std::cout << "[EpollServer] Received: " << buffer << std::endl;
                        send(current_fd, buffer, bytes_read, 0);
                    }
                    else if (bytes_read == 0)
                    {
                        // 客户端连接已断开
                        std::cout << "[Server] Client disconnected, fd: " << current_fd << std::endl;
                        close(current_fd);
                        break;
                    }
                    else if (bytes_read == -1 && errno == EINTR)
                    {
                        // 读取数据的时候被信号中断，继续读取
                        continue;
                    }
                    else if (bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                    {
                        // 全部数据已读取完毕
                        break;
                    }
                }
            }
            else if (current_event.events & EPOLLOUT)
            {
                // 写事件准备好
            }
            else
            {
                // 其他事件都认为是错误
                std::cout << "[Server] unknow event: " << current_event.events << "for client: " << current_event.data.
                    fd << std::endl;
                close(current_fd);
            }
        }
    }

    close(server_socket_fd);
    close(epoll_fd);
    return 0;
}
