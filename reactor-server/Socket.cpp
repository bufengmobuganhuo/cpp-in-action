//
// Created by yuzhang on 2026/8/12.
//

#include "include/Socket.h"

#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include "include/InetAddress.h"

int create_non_blocking()
{
    int client_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (client_fd < 0)
    {
        perror("[Server] socket creation failed");
        exit(-1);
    }
    return client_fd;
}

Socket::Socket(int fd): fd_(fd), port_(0)
{}

Socket::Socket(int fd, const std::string& ip, uint16_t port): fd_(fd), ip_(ip), port_(port)
{}

Socket::~Socket()
{
    close(fd_);
}

int Socket::fd() const
{
    return fd_;
}

std::string Socket::ip() const
{
    return ip_;
}

uint16_t Socket::port() const
{
    return port_;
}

void Socket::set_reuse_addr(bool on) const
{
    int opt = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
}

void Socket::set_reuse_port(bool on) const
{
    int opt = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof opt);
}

void Socket::set_tcp_no_delay(bool on) const
{
    int opt = on ? 1: 0;
    setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof opt);
}

void Socket::set_keep_alive(bool on) const
{
    int opt = on ? 1: 0;
    setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof opt);
}

void Socket::bind(const InetAddress& serv_addr)
{
    if (::bind(fd_, serv_addr.addr(), sizeof(sockaddr_in)) < 0)
    {
        perror("[Server] bind failed");
        close(fd_);
        exit(-1);
    }
    ip_ = serv_addr.ip();
    port_= serv_addr.port();
}

void Socket::listen(int nn) const
{
    // 将 socket 设置为监听状态，等待客户端连接
    // 第二个参数 backlog 指定内核中该 socket 对应挂起连接队列的最大长度
    // 128 是常见的经验值，表示允许最多 128 个已完成三次握手但尚未被 accept() 取走的连接排队等待
    // 注意：实际值可能会被内核调整（如 Linux 中会自动向上取整到下一个 2 的幂），且受系统参数 somaxconn 限制
    if (::listen(fd_, nn) < 0)
    {
        perror("[Server] listen failed");
        exit(-1);
    }
}

int Socket::accept(InetAddress& client_addr)
{
    struct sockaddr_in peer_addr{};
    socklen_t addr_len = sizeof(peer_addr);
    int new_socket = accept4(fd_, (struct sockaddr*)&peer_addr, &addr_len,
                             SOCK_NONBLOCK);
    client_addr.set_addr(peer_addr);
    return new_socket;
}
