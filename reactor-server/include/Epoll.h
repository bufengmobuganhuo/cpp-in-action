//
// Created by yuzhang on 2026/8/16.
//

#ifndef REACTOR_SERVER_EPOLL_H
#define REACTOR_SERVER_EPOLL_H

#include <vector>
#include <sys/epoll.h>

class Channel;

constexpr int kMaxEvents = 100;

class Epoll
{
private:
    int epollfd_ = -1;
    epoll_event readyEvents_[kMaxEvents]{}; // 存储epoll_wait()返回的事件
public:
    Epoll();
    ~Epoll();

    void updateChannel(Channel *channel) const; // 把Channel添加/更新到Epoll实例
    std::vector<Channel*> loop(int timeout= -1); // 运行epoll_wait()，等待就绪的事件
};

#endif //REACTOR_SERVER_EPOLL_H
