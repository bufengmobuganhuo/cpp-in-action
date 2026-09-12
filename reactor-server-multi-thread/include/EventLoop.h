//
// Created by yuzhang on 2026/8/26.
//

#ifndef REACTOR_SERVER_EVENTLOOP_H
#define REACTOR_SERVER_EVENTLOOP_H
#include <functional>
#include <memory>
#include <queue>
#include <mutex>
#include <thread>
#include <sys/timerfd.h>
#include <map>
#include <atomic>

class Epoll;
class Channel;
class Connection;

class EventLoop
{
private:
    std::unique_ptr<Epoll> epoll_;
    std::atomic_bool is_stopped;
    std::function<void(EventLoop*)> on_timeout_callback_func_; // epoll_wait超时的回调
    long thread_id; // 启动事件循环所在线程的id，就是IO线程的id
    std::queue<std::function<void()>> task_que_;
    std::mutex task_mutex_;
    int wakeup_fd_;
    std::unique_ptr<Channel> wakeup_channel;
    int timerfd_;
    std::unique_ptr<Channel> timer_channel_;
    bool is_main_loop_; // 表示是否为主事件循环
    std::mutex conns_mutex_;
    std::map<int, std::shared_ptr<Connection>> conns_; // <fd, connection>
    std::function<void(int)> on_remove_conn_callback_func_; // 移除Connection的回调
public:
    EventLoop(bool is_main_loop);
    ~EventLoop();

    void run(); // 运行事件循环
    void stop();
    void update_channel(Channel *ch);// 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
    void remove_channel(Channel *ch);// 删除监听的channel
    void set_on_timeout_callback_func_(std::function<void(EventLoop*)> on_timeout_callback_func);
    bool is_io_thread(); // 判断当前线程是否为启动事件循环的线程
    void enqueue(std::function<void()>);
    void wakeup() const; // 唤醒事件循环
    void handle_wakeup();
    void handle_timer();
    void new_connection(std::shared_ptr<Connection> conn);
    void set_on_remove_conn_callback_func_(std::function<void(int)> on_remove_conn_callback_func);
};

#endif //REACTOR_SERVER_EVENTLOOP_H
