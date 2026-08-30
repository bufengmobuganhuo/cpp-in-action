//
// Created by yuzhang on 2026/8/18.
//
#include "include/Channel.h"

#include <iostream>
#include <strings.h>
#include <unistd.h>

#include "include/Epoll.h"
#include "include/EventLoop.h"
#include "include/InetAddress.h"
#include "include/Socket.h"

Channel::Channel(EventLoop* event_loop, int fd) : eventLoop_(event_loop), fd_(fd)
{
}

Channel::~Channel()
= default;

int Channel::fd() const
{
    return fd_;
}

void Channel::useET()
{
    events_ = events_ | EPOLLET;
}

void Channel::enableReading()
{
    events_ = events_ | EPOLLIN;
    eventLoop_->updateChannel(this);
}

void Channel::setInEpoll()
{
    inEpoll_ = true;
}

void Channel::setReadyEvent(uint32_t event)
{
    readyEvents_ = event;
}

bool Channel::inEpoll() const
{
    return inEpoll_;
}

uint32_t Channel::events() const
{
    return events_;
}

uint32_t Channel::readyEvents() const
{
    return readyEvents_;
}

void Channel::handleEvent()
{
    if (events_ & EPOLLRDHUP)
    {

    }
    else if (events_ & (EPOLLIN | EPOLLPRI))
    {
        readCallback_();
    }
    else if (events_ & EPOLLOUT)
    {
        // 写事件准备好
    }
    else
    {

    }
}

void Channel::onMessage() const
{
    // 客户端有数据可读，要一直读完为止
    // 处理客户端数据
    char buffer[kBufferSize] = {0};
    while (true)
    {
        // 清理buffer
        bzero(&buffer, sizeof(buffer));
        int bytes_read = read(fd_, buffer, kBufferSize - 1);
        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            std::cout << "[EpollServer] Received: " << buffer << std::endl;
            send(fd_, buffer, bytes_read, 0);
        }
        else if (bytes_read == 0)
        {
            // 客户端连接已断开
            std::cout << "[Server] Client disconnected, fd: " << fd_ << std::endl;
            close(fd_);
            break;
        }
        else if (bytes_read == -1 && errno == EINTR)
        {
            // 读取数据的时候被信号中断，继续读取
            continue;
        }
        else if (bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            // 全部数据已读取完毕
            break;
        }
    }
}

void Channel::setReadCallback(std::function<void()> readCallback)
{
    readCallback_ = readCallback;
}
