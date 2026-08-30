//
// Created by yuzhang on 2026/8/18.
//
#include "include/Channel.h"

#include <iostream>
#include <utility>
#include <strings.h>
#include <unistd.h>

#include "include/Epoll.h"
#include "include/EventLoop.h"
#include "include/InetAddress.h"
#include "include/Socket.h"

Channel::Channel(EventLoop* event_loop, int fd) : event_loop_(event_loop), fd_(fd)
{
}

Channel::~Channel()
= default;

int Channel::fd() const
{
    return fd_;
}

void Channel::use_et()
{
    events_ = events_ | EPOLLET;
}

void Channel::enable_reading()
{
    events_ = events_ | EPOLLIN;
    event_loop_->update_channel(this);
}

void Channel::set_in_epoll()
{
    in_epoll_ = true;
}

void Channel::set_ready_event(uint32_t event)
{
    ready_events_ = event;
}

bool Channel::in_epoll() const
{
    return in_epoll_;
}

uint32_t Channel::events() const
{
    return events_;
}

uint32_t Channel::ready_events() const
{
    return ready_events_;
}

void Channel::handle_event()
{
    if (events_ & EPOLLRDHUP)
    {
        close_callback_();
    }
    else if (events_ & (EPOLLIN | EPOLLPRI))
    {
        read_callback_();
    }
    else if (events_ & EPOLLOUT)
    {
        // 写事件准备好
    }
    else
    {
        error_callback_();
    }
}

void Channel::on_message() const
{
    // 客户端有数据可读，要一直读完为止
    // 处理客户端数据
    char buffer[k_buffer_size] = {0};
    while (true)
    {
        // 清理buffer
        bzero(&buffer, sizeof(buffer));
        int bytes_read = read(fd_, buffer, k_buffer_size - 1);
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

void Channel::set_read_callback(std::function<void()> read_callback)
{
    read_callback_ = std::move(read_callback);
}

void Channel::set_close_callback(std::function<void()> fn)
{
    close_callback_ = std::move(fn);
}

void Channel::set_error_callback(std::function<void()> fn)
{
    error_callback_ = std::move(fn);
}
