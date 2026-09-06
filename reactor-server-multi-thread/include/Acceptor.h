//
// Created by yuzhang on 2026/8/27.
//

#ifndef REACTOR_SERVER_ACCEPTOR_H
#define REACTOR_SERVER_ACCEPTOR_H
#include <functional>
#include <string>
#include <memory>

#include "Socket.h"

class EventLoop;
class Socket;
class Channel;

class Acceptor
{
private:
    const std::unique_ptr<EventLoop>& event_loop_; // Acceptor对应的事件循环，属于TcpServer，在构造函数中传入
    Socket* serv_socket_; // 服务端用于监听的socket，在构造函数中创建
    Channel* accept_channel_; // 链接建立后创建的服务端和客户端之间的channel，在构造函数中创建
    std::function<void(std::unique_ptr<Socket>)> new_connection_func_; // 创建Connection的回调函数
public:
    Acceptor(const std::unique_ptr<EventLoop>& event_loop, const std::string& ip, uint16_t port);
    ~Acceptor();
    void new_connection() const; // 创建Connection
    void set_new_connection_func(std::function<void(std::unique_ptr<Socket>)> fn);
};

#endif //REACTOR_SERVER_ACCEPTOR_H
