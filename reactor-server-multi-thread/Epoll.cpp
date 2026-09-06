//
// Created by yuzhang on 2026/8/16.
//
#include "include/Epoll.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <strings.h>
#include <unistd.h>

#include "include/Channel.h"

Epoll::Epoll()
{
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0)
    {
        perror("[Server] epoll_create failed");
        exit(-1);
    }
}

Epoll::~Epoll()
{
    close(epoll_fd_);
}

void Epoll::update_channel(Channel* channel) const
{
    epoll_event ev{};
    // 指定channel
    ev.data.ptr = channel;
    // 指定事件
    ev.events = channel->events();
    if (channel->in_epoll()) // 如果已经在Epoll实例中，则更新
    {
        // 使用 epoll_ctl 将服务端的监听 socket 注册到 epoll 实例中
        // 参数 1: epoll_fd - epoll 实例的文件描述符，由 epoll_create 创建
        // 参数 2: EPOLL_CTL_ADD - 控制命令，表示向 epoll 实例中添加一个新的文件描述符（其他命令有 EPOLL_CTL_MOD 修改, EPOLL_CTL_DEL 删除）
        // 参数 3: server_socket_fd - 需要被监听的文件描述符，即服务端的 listen fd
        // 参数 4: &server_event - 指向 epoll_event 结构体的指针，指定了要监听的事件类型（如 EPOLLIN）及关联的用户数据（如 data.fd）
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, channel->fd(), &ev) < 0)
        {
            perror("[EpollServer] epoll_ctl MOD server failed");
            close(channel->fd());
            close(epoll_fd_);
            exit(-1);
        }
    }
    else // 不在Epoll实例中，则添加
    {
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, channel->fd(), &ev) < 0)
        {
            perror("[EpollServer] epoll_ctl ADD server failed");
            close(channel->fd());
            close(epoll_fd_);
            exit(-1);
        }
        channel->set_in_epoll();
    }
}

void Epoll::remove_channel(Channel* channel)
{
    epoll_event ev{};
    // 指定channel
    ev.data.ptr = channel;
    // 指定事件
    ev.events = channel->events();
    if (channel->in_epoll())
    {
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, channel->fd(), &ev) < 0)
        {
            perror("[EpollServer] epoll_ctl DEL server failed");
            return;
        }
        printf("removed channel: %d", channel->fd());
    }
}

std::vector<Channel*> Epoll::loop(int timeout)
{
    std::vector<Channel*> channels;
    bzero(ready_events_, sizeof ready_events_);
    int ready_count = epoll_wait(epoll_fd_, ready_events_, k_max_events, timeout);
    if (ready_count < 0)
    {
        perror("[Server] epoll_wait failed");
    }
    if (ready_count == 0)
    {
        return channels; // 超时直接返回
    }
    for (int i = 0; i < ready_count; i++)
    {
        auto* channel = static_cast<Channel*>(ready_events_[i].data.ptr);
        channel->set_ready_event(ready_events_[i].events);
        channels.push_back(channel);
    }
    return channels; // 返回就绪的事件
}
