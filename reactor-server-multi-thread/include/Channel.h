//
// Created by yuzhang on 2026/8/18.
//

#ifndef REACTOR_SERVER_CHANNEL_H
#define REACTOR_SERVER_CHANNEL_H

#include <arpa/inet.h>
#include <functional>
#include <memory>

// epoll_wait每次最多返回的就绪事件数
constexpr int k_buffer_size = 1024;

class EventLoop;
class Socket;
class InetAddress;

class Channel
{
private:
    int fd_ = -1; // channel和fd一一对应
    const std::unique_ptr<EventLoop>& event_loop_; // Channel对应的Epoll实例，二者同样一一对应
    bool in_epoll_ = false; // Channel是否已添加到Epoll实例，如果未添加，调用epoll_ctl()时使用ADD指令，否则用MOD指令
    uint32_t events_ = 0; // fd_需要监视的事件，serv_fd/client_fd需要监听EPOLLIN事件，client_fd还需要监听EPOLLOUT事件
    uint32_t ready_events_ = 0; // 已经就绪的事件
    std::function<void()> read_callback_; // fd_读事件的回调函数
    std::function<void()> write_callback_; // fd_写事件的回调函数
    std::function<void()> disconnect_callback_; // 连接关闭的回调
    std::function<void()> error_callback_;
public:
    Channel(const std::unique_ptr<EventLoop>& epoll, int fd);
    ~Channel();

    int fd() const;
    void use_et(); // 采用边缘触发模式
    void enable_reading(); // 让epoll_wait()监听fd_的读事件
    void disable_reading(); // 取消
    void enable_writing(); // 让epoll_wait()监听fd_的写事件
    void disable_writing(); // 取消
    void disable_all(); // 取消全部监听
    void remove_channel(); // 从事件循环中删除channel
    void set_in_epoll(); // in_epoll_ = true, 表示已添加到Epoll实例
    void set_ready_event(uint32_t event); // 设置已就绪事件
    bool in_epoll() const; // 是否已就绪
    uint32_t events() const; // 要监听的事件
    uint32_t ready_events() const; // 已就绪事件

    void handle_event(); // 处理已就绪事件

    void set_read_callback(std::function<void()> read_callback); // 设置回调函数
    void set_write_callback(std::function<void()> write_callback);
    void set_close_callback(std::function<void()> fn);
    void set_error_callback(std::function<void()> fn);
};

#endif //REACTOR_SERVER_CHANNEL_H
