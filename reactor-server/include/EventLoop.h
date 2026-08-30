//
// Created by yuzhang on 2026/8/26.
//

#ifndef REACTOR_SERVER_EVENTLOOP_H
#define REACTOR_SERVER_EVENTLOOP_H

class Epoll;
class Channel;

class EventLoop
{
private:
    Epoll* epoll_;
public:
    EventLoop();
    ~EventLoop();

    void run(); // 运行事件循环
    void updateChannel(Channel *ch);// 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
};

#endif //REACTOR_SERVER_EVENTLOOP_H
