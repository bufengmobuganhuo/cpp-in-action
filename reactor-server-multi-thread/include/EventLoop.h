//
// Created by yuzhang on 2026/8/26.
//

#ifndef REACTOR_SERVER_EVENTLOOP_H
#define REACTOR_SERVER_EVENTLOOP_H
#include <functional>

class Epoll;
class Channel;

class EventLoop
{
private:
    Epoll* epoll_;
    std::function<void(EventLoop*)> on_timeout_callback_func_; // epoll_wait超时的回调
public:
    EventLoop();
    ~EventLoop();

    void run(); // 运行事件循环
    void update_channel(Channel *ch);// 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
    void set_on_timeout_callback_func_(std::function<void(EventLoop*)> on_timeout_callback_func);
};

#endif //REACTOR_SERVER_EVENTLOOP_H
