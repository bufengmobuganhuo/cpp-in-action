//
// Created by yuzhang on 2026/8/12.
//

#ifndef REACTOR_SERVER_SOCKET_H
#define REACTOR_SERVER_SOCKET_H

#include "InetAddress.h"

int createNonBlocking();

class Socket
{
private:
    const int fd_;
public:
    Socket(int fd);
    ~Socket();

    int fd() const;
    void setReuseAddr(bool on) const; // 设置SO_REUSEADDR选项
    void setReusePort(bool on) const; // 设置SO_REUSEPORT选项
    void setTcpNoDelay(bool on) const; // 设置TCP_NODELAY选项
    void setKeepAlive(bool on) const; // 设置SO_KEEPALIVE选项
    void bind(const InetAddress& serv_addr) const; // 绑定IP和端口号
    void listen(int nn=128) const; // 服务端开启监听
    int accept(InetAddress& client_addr) const; // 接受客户端连接请求
};


#endif //REACTOR_SERVER_SOCKET_H
