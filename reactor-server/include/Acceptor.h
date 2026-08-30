//
// Created by yuzhang on 2026/8/27.
//

#ifndef REACTOR_SERVER_ACCEPTOR_H
#define REACTOR_SERVER_ACCEPTOR_H
#include <functional>
#include <string>

#include "Socket.h"

class EventLoop;
class Socket;
class Channel;

class Acceptor
{
private:
    EventLoop* eventLoop_; // Acceptor对应的事件循环，属于TcpServer，在构造函数中传入
    Socket* servSocket_; // 服务端用于监听的socket，在构造函数中创建
    Channel* acceptChannel_; // 链接建立后创建的服务端和客户端之间的channel，在构造函数中创建
    std::function<void(Socket*)> newConnectionFunc_; // 创建Connection的回调函数
public:
    Acceptor(EventLoop* eventLoop_, const std::string& ip, uint16_t port);
    ~Acceptor();
    void newConnection() const; // 创建Connection
    void setNewConnectionFunc(std::function<void(Socket*)> fn);

};

#endif //REACTOR_SERVER_ACCEPTOR_H
