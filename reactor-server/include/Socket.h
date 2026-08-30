//
// Created by yuzhang on 2026/8/12.
//

#ifndef REACTOR_SERVER_SOCKET_H
#define REACTOR_SERVER_SOCKET_H
#include <cstdint>
#include <stdio.h>
#include <string>

class InetAddress;

int createNonBlocking();

class Socket
{
private:
    const int fd_;
    std::string ip_; // 如果是listenFd，则存放服务端监听的ip，如果时客户端连接的fd，则存放客户端的ip
    uint16_t port_; // 同上
public:
    Socket(int fd);
    Socket(int fd, const std::string& ip, uint16_t port);
    ~Socket();

    int fd() const;
    std::string ip() const;
    uint16_t port() const;
    void setReuseAddr(bool on) const; // 设置SO_REUSEADDR选项
    void setReusePort(bool on) const; // 设置SO_REUSEPORT选项
    void setTcpNoDelay(bool on) const; // 设置TCP_NODELAY选项
    void setKeepAlive(bool on) const; // 设置SO_KEEPALIVE选项
    void bind(const InetAddress& serv_addr); // 绑定IP和端口号
    void listen(int nn=128) const; // 服务端开启监听
    int accept(InetAddress& client_addr); // 接受客户端连接请求
};


#endif //REACTOR_SERVER_SOCKET_H
