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
class Buffer;

class Connection
{
private:
    EventLoop* event_loop_; // Connection对应的事件循环
    Socket* client_socket_; // 与客户端通讯的socket
    Channel* client_channel_; // Connection对应的channel
    Buffer* input_buffer_; // 接收缓冲区
    Buffer* output_buffer_; // 发送缓冲区
    std::function<void(Connection*, std::string)> on_message_callback_func_; // TcpServer的回调
    std::function<void(Connection*)> on_write_complete_callback_func_; // TcpServer的回调
    std::function<void(Connection*)> on_disconnect_callback_func_; // TcpServer的回调
    std::function<void(Connection*)> on_error_callback_func_; // TcpServer的回调
public:
    Connection(EventLoop* event_loop, Socket* socket);
    ~Connection();
    int fd() const;
    std::string ip() const;
    uint16_t port() const;
    void on_message(); // 处理对端发送过来的消息，放入接收缓冲区
    void on_disconnect(); // TCP链接断开
    void on_error(); // TCP链接报错
    void set_on_message_callback_func_(std::function<void(Connection*, std::string)> on_message_callback_func);
    void set_on_write_complete_callback_func_(std::function<void(Connection*)> on_write_complete_callback_func);
    void set_on_disconnect_callback_func(std::function<void(Connection*)> on_disconnect_callback_func);
    void set_on_error_callback_func(std::function<void(Connection*)> on_error_func);
    void send(const char* data, size_t size); // 把要发送的数据放到发送缓冲区
    void write(); // 写事件准备好时，将发送缓冲区的数据发出去
};

#endif //REACTOR_SERVER_CONNECTION_H
