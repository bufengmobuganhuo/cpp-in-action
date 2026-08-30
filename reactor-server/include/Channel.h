//
// Created by yuzhang on 2026/8/18.
//

#ifndef REACTOR_SERVER_CHANNEL_H
#define REACTOR_SERVER_CHANNEL_H

#include <arpa/inet.h>
#include <functional>

// epoll_wait每次最多返回的就绪事件数
constexpr int kBufferSize = 1024;

class EventLoop;
class Socket;
class InetAddress;

class Channel
{
private:
    int fd_ = -1; // channel和fd一一对应
    EventLoop* eventLoop_ = nullptr; // Channel对应的Epoll实例，二者同样一一对应
    bool inEpoll_ = false; // Channel是否已添加到Epoll实例，如果未添加，调用epoll_ctl()时使用ADD指令，否则用MOD指令
    uint32_t events_ = 0; // fd_需要监视的事件，serv_fd/client_fd需要监听EPOLLIN事件，client_fd还需要监听EPOLLOUT事件
    uint32_t readyEvents_ = 0; // 已经就绪的事件
    std::function<void()> readCallback_; // fd_读事件的回调函数
    std::function<void()> closeCallback_; // 连接关闭的回调
    std::function<void()> errorCallback_;
public:
    Channel(EventLoop* epoll, int fd);
    ~Channel();

    int fd() const;
    void useET(); // 采用边缘触发模式
    void enableReading(); // 让epoll_wait()监听fd_的读事件
    void setInEpoll(); // inEpoll_ = true, 表示已添加到Epoll实例
    void setReadyEvent(uint32_t event); // 设置已就绪事件
    bool inEpoll() const; // 是否已就绪
    uint32_t events() const; // 要监听的事件
    uint32_t readyEvents() const; // 已就绪事件

    void handleEvent(); // 处理已就绪事件

    void onMessage() const; // 处理对端发送过来的消息
    void setReadCallback(std::function<void()> readCallback); // 设置回调函数
};

#endif //REACTOR_SERVER_CHANNEL_H
