//
// Created by yuzhang on 2026/8/26.
//
#include "include/EventLoop.h"

#include <cstring>
#include <utility>

#include "include/Channel.h"
#include "include/Epoll.h"
#include <sys/eventfd.h>
#include <syscall.h>
#include <unistd.h>
#include <set>

#include "include/Connection.h"

int create_timerfd(int sec=30)
{
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    struct itimerspec timeout;
    memset(&timeout, 0, sizeof(struct itimerspec));
    timeout.it_value.tv_sec = sec; // 5秒超时
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(timerfd, 0, &timeout, 0);
    return timerfd;
}

EventLoop::EventLoop(bool is_main_loop):
epoll_(new Epoll()), wakeup_fd_(eventfd(0, EFD_NONBLOCK)),
wakeup_channel(new Channel(this, wakeup_fd_)), timerfd_(create_timerfd()), timer_channel_(new Channel(this, timerfd_)),
is_main_loop_(is_main_loop)
{
    wakeup_channel->set_read_callback(std::bind(&EventLoop::handle_wakeup, this));
    wakeup_channel->enable_reading();

    timer_channel_->set_read_callback(std::bind(&EventLoop::handle_timer, this));
    timer_channel_->enable_reading();
}

EventLoop::~EventLoop()
{
}

void EventLoop::run()
{
    is_stopped = false;
    thread_id = syscall(SYS_gettid);
    while (!is_stopped)
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

void EventLoop::stop()
{
    is_stopped = true;
    wakeup(); // 需要唤醒事件循环
}

void EventLoop::update_channel(Channel* ch)
{
    epoll_->update_channel(ch);
}

void EventLoop::remove_channel(Channel* ch)
{
    epoll_->remove_channel(ch);
}

void EventLoop::set_on_timeout_callback_func_(std::function<void(EventLoop*)> on_timeout_callback_func)
{
    on_timeout_callback_func_ = std::move(on_timeout_callback_func);
}

bool EventLoop::is_io_thread()
{
    return thread_id == syscall(SYS_gettid);
}

void EventLoop::enqueue(std::function<void()> fn)
{
    {
        std::lock_guard<std::mutex> gd(task_mutex_);
        task_que_.push(fn);
    }
    // 唤醒事件循环
    wakeup();
}

void EventLoop::wakeup() const
{
    uint64_t val = 1;
    write(wakeup_fd_, &val, sizeof val);
}

void EventLoop::handle_wakeup()
{
    printf("handle_wakeup, thread is %ld\n", syscall(SYS_gettid));
    uint64_t val;
    read(wakeup_fd_, &val, sizeof val); // 从eventfd中读取事件，目的是清理这个唤醒信号
    {
        std::lock_guard<std::mutex> gd(task_mutex_);
        while (!task_que_.empty())
        {
            std::function<void()> fn = std::move(task_que_.front());
            task_que_.pop();
            fn(); // 执行任务
        }
    }
}

void EventLoop::handle_timer()
{
    struct itimerspec timeout;
    memset(&timeout, 0, sizeof(struct itimerspec));
    timeout.it_value.tv_sec = 5; // 5秒超时
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(timerfd_, 0, &timeout, 0);
    if (is_main_loop_)
    {
    }
    else
    {
        // 检查Connection是否生效
        time_t now = time(0);
        std::set<int> fd_to_erase;
        for (const auto& pair : conns_)
        {
            auto conn = pair.second;
            if (conn.get()->is_idle_timeout(now, 10))
            {
                printf("connection %d is idle timeout\n", conn.get()->fd());
                fd_to_erase.insert(conn.get()->fd());
            }
        }
        {
            std::lock_guard<std::mutex> gd(conns_mutex_);
            for (auto fd : fd_to_erase)
            {
                conns_.erase(fd);
                on_remove_conn_callback_func_(fd);
            }
        }
    }
}

void EventLoop::new_connection(std::shared_ptr<Connection> conn)
{
    std::lock_guard<std::mutex> gd(conns_mutex_);
    conns_[conn.get()->fd()] = conn;
}

void EventLoop::set_on_remove_conn_callback_func_(std::function<void(int)> on_remove_conn_callback_func)
{
    on_remove_conn_callback_func_ = std::move(on_remove_conn_callback_func);
}
