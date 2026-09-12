//
// Created by yuzhang on 2026/9/3.
//
#include "include/ThreadPool.h"

#include <iostream>
#include <unistd.h>
#include <syscall.h>


ThreadPool::ThreadPool(size_t thread_num, std::string thread_name): stop_(false), thread_name_(std::move(thread_name))
{
    for (int i = 0; i < thread_num; i++)
    {
        threads_.emplace_back([this]
        {
            printf("[%s]create thread(%ld).\n", thread_name_.data(), syscall(SYS_gettid)); // 打印Linux系统tid
            std::cout << thread_name_ << "子线程： " << std::this_thread::get_id() << std::endl; // 打印C++标准库的tid

            while (true)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->mutex_); // 加锁
                    this->condition_variable_.wait(lock, [this]
                    {
                        return this->stop_ || !this->task_que_.empty();// 线程结束等待的条件：只要线程池正在停止 或 队列不为空。
                    }
                    );

                    if (this->stop_ && this->task_que_.empty())
                    {
                        return;// 如果线程池在停止 && 队列为空，则可以直接停止线程
                    }
                    task = std::move(this->task_que_.front());
                    this->task_que_.pop();// 取出任务
                }
                printf("[%s] thread is %ld.\n", thread_name_.data(), syscall(SYS_gettid));
                task(); // 执行任务
            }
        }
        );
    }
}

void ThreadPool::add_task(const std::function<void()>& fn)
{
    if (stop_)
    {
        printf("[%s]ThreadPool is stopping...\n", thread_name_.data());
    }
    {
        std::unique_lock<std::mutex> lock(this->mutex_); // 加锁
        task_que_.push(fn);
    }
    condition_variable_.notify_one();
}

size_t ThreadPool::thread_size()
{
    return threads_.size();
}

void ThreadPool::shutdown()
{
    stop_ = true;
    condition_variable_.notify_all();
    for (std::thread &thread : threads_)
    {
        thread.join(); // 等待所有任务结束后退出
    }
}

ThreadPool::~ThreadPool()
{
    shutdown();
}


