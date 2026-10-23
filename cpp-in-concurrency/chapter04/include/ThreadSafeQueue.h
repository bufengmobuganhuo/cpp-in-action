//
// Created by zhangyu on 2026/10/12.
//

#pragma once
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

#include "../../chapter02/include/ThreadSafeStack.h"

template<typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue();
    ThreadSafeQueue(const ThreadSafeQueue&);
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete; // 禁用赋值

    void push(T new_value);

    bool try_pop(T& value);
    std::shared_ptr<T> try_pop();

    void wait_and_pop(T& value);
    std::shared_ptr<T> wait_and_pop();

    bool empty() const;
private:
    mutable std::mutex mutex_;
    std::condition_variable condition_variable_;
    std::queue<T> queue_;
};
