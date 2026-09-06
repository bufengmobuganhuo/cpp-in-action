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

Channel::Channel(const std::unique_ptr<EventLoop>& event_loop, int fd) : event_loop_(event_loop), fd_(fd)
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

void Channel::disable_reading()
{
    events_ &= ~EPOLLIN;
    event_loop_->update_channel(this);
}

void Channel::enable_writing()
{
    events_ |= EPOLLOUT;
    event_loop_->update_channel(this);
}

void Channel::disable_writing()
{
    events_ &= ~EPOLLOUT;
    event_loop_->update_channel(this);
}

void Channel::disable_all()
{
    events_ = 0;
    event_loop_->update_channel(this);
}

void Channel::remove_channel()
{
    disable_all();
    event_loop_->remove_channel(this);
    in_epoll_ = false;
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
    if (ready_events_ & EPOLLRDHUP)
    {
        remove_channel();
        disconnect_callback_();
    }
    else if (ready_events_ & (EPOLLIN | EPOLLPRI))
    {
        read_callback_();
    }
    else if (ready_events_ & EPOLLOUT)
    {
        // 写事件准备好
        write_callback_();
    }
    else
    {
        remove_channel();
        error_callback_();
    }
}

void Channel::set_read_callback(std::function<void()> read_callback)
{
    read_callback_ = std::move(read_callback);
}

void Channel::set_write_callback(std::function<void()> write_callback)
{
    write_callback_ = write_callback;
}

void Channel::set_close_callback(std::function<void()> fn)
{
    disconnect_callback_ = std::move(fn);
}

void Channel::set_error_callback(std::function<void()> fn)
{
    error_callback_ = std::move(fn);
}
