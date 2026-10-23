//
// Created by zhangyu on 2026/10/12.
//

#include "include/ThreadSafeQueue.h"

template <typename T>
ThreadSafeQueue<T>::ThreadSafeQueue()
= default;

template <typename T>
ThreadSafeQueue<T>::ThreadSafeQueue(const ThreadSafeQueue& thread_safe_queue)
{
    std::lock_guard<std::mutex> lock(thread_safe_queue.mutex_);
    queue_ = thread_safe_queue.queue_;
}

template <typename T>
void ThreadSafeQueue<T>::push(T new_value)
{
    std::lock_guard<std::mutex> lock_guard(mutex_);
    queue_.push(new_value);
    condition_variable_.notify_all();
}

template <typename T>
bool ThreadSafeQueue<T>::try_pop(T& value)
{
    std::lock_guard<std::mutex> lock_guard(mutex_);
    if (queue_.empty())
    {
        return false;
    }
    value = queue_.front();
    queue_.pop();
    return true;
}

template <typename T>
std::shared_ptr<T> ThreadSafeQueue<T>::try_pop()
{
    std::lock_guard<std::mutex> lock_guard(mutex_);
    if (queue_.empty())
    {
        return std::shared_ptr<T>();
    }
    std::shared_ptr<T> value(std::make_shared<T>(queue_.front()));
    return value;
}

template <typename T>
void ThreadSafeQueue<T>::wait_and_pop(T& value)
{
    std::unique_lock<std::mutex> unique_lock(mutex_);
    condition_variable_.wait(unique_lock, [this]{return !queue_.empty();});
    value = queue_.front();
    queue_.pop();
}

template <typename T>
std::shared_ptr<T> ThreadSafeQueue<T>::wait_and_pop()
{
    std::unique_lock<std::mutex> unique_lock(mutex_);
    condition_variable_.wait(unique_lock, [this]{return !queue_.empty();});
    std::shared_ptr<T> value(std::make_shared<T>(queue_.front()));
    queue_.pop();
    return value;
}

template <typename T>
bool ThreadSafeQueue<T>::empty() const
{
    std::lock_guard<std::mutex> lock_guard(mutex_);
    return queue_.empty();
}
