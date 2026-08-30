//
// Created by yuzhang on 2026/8/30.
//

#ifndef REACTOR_SERVER_CONNECTION_H
#define REACTOR_SERVER_CONNECTION_H

#include <cstdint>
#include <string>
#include <functional>

class EventLoop;
class Socket;
class Channel;

class Connection
{
private:
    EventLoop* event_loop_; // Connection对应的事件循环
    Socket* client_socket_; // 与客户端通讯的socket
    Channel* client_channel_; // Connection对应的channel
    std::function<void(Connection*)> on_disconnect_func_;
    std::function<void(Connection*)> on_error_func_;
public:
    Connection(EventLoop* event_loop, Socket* socket);
    ~Connection();
    int fd() const;
    std::string ip() const;
    uint16_t port() const;
    void on_disconnect(); // TCP链接断开
    void on_error(); // TCP链接报错
    void set_on_disconnect_func(std::function<void(Connection*)> on_close_func);
    void set_on_error_func(std::function<void(Connection*)> on_error_func);
};

#endif //REACTOR_SERVER_CONNECTION_H
