//
// Created by yuzhang on 2026/8/26.
//
#include <utility>

#include "include/EventLoop.h"

#include "include/Channel.h"
#include "include/Epoll.h"

EventLoop::EventLoop():epoll_(new Epoll())
{

}

EventLoop::~EventLoop()
{
    delete epoll_;
}

void EventLoop::run()
{
    while (true)
    {
        std::vector<Channel*> ready_channels = epoll_->loop();
        if (ready_channels.empty())
        {
            on_timeout_callback_func_(this);
        }
        // 【核心区别】与其他的核心区别，只遍历就绪的事件
        for (auto channel : ready_channels)
        {
            channel->handle_event();
        }
    }
}

void EventLoop::update_channel(Channel* ch)
{
    epoll_->update_channel(ch);
}

void EventLoop::set_on_timeout_callback_func_(std::function<void(EventLoop*)> on_timeout_callback_func)
{
    on_timeout_callback_func_ = std::move(on_timeout_callback_func);
}
