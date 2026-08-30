//
// Created by yuzhang on 2026/8/26.
//
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
        // 【核心区别】与其他的核心区别，只遍历就绪的事件
        for (auto channel : ready_channels)
        {
            channel->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel* ch)
{
    epoll_->updateChannel(ch);
}
