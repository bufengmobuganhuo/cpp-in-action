//
// Created by yuzhang on 2026/8/30.
//

#ifndef REACTOR_SERVER_CONNECTION_H
#define REACTOR_SERVER_CONNECTION_H

#include <cstdint>
#include <string>

class EventLoop;
class Socket;
class Channel;

class Connection
{
private:
    EventLoop* eventLoop_; // Connection对应的事件循环
    Socket* clientSocket_; // 与客户端通讯的socket
    Channel* clientChannel_; // Connection对应的channel
public:
    Connection(EventLoop* eventLoop, Socket* socket);
    ~Connection();
    int fd() const;
    std::string ip() const;
    uint16_t port() const;
    void on_close() const; // TCP链接断开
    void on_error(uint32_t unknownEvents) const; // TCP链接报错
};

#endif //REACTOR_SERVER_CONNECTION_H
