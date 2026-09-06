//
// Created by yuzhang on 2026/9/3.
//

#ifndef REACTOR_SERVER_THREADPOOL_H
#define REACTOR_SERVER_THREADPOOL_H
#include <atomic>
#include <condition_variable>
#include <queue>
#include <thread>
#include <vector>
#include <functional>
#include <mutex>

class ThreadPool
{
private:
    std::vector<std::thread> threads_; // 线程池中的线程
    std::queue<std::function<void()>> task_que_; // 任务队列
    std::mutex mutex_; // 任务队列的同步互斥锁
    std::condition_variable condition_variable_; // 任务队列同步的条件变量
    std::atomic_bool stop_; // 线程池停止标志位
    std::string thread_name_;
public:
    ThreadPool(size_t thread_num, std::string thread_name_);
    void add_task(const std::function<void()>& fn); // 添加任务

    ~ThreadPool();
};
#endif //REACTOR_SERVER_THREADPOOL_H
