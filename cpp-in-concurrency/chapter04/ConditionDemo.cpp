//
// Created by zhangyu on 2026/10/12.
//
#include <condition_variable>
#include <mutex>
#include <queue>

struct data
{

};
std::mutex mutex_;
std::queue<data> queue_;
std::condition_variable condition_;

bool has_more_data()
{
    return true;
};

void data_preparation_thread()
{
    while (has_more_data())
    {
        data prepared_data;
        {
            std::lock_guard<std::mutex> lock(mutex_); // 入队前需要获取锁
            queue_.push(prepared_data);
        }
        condition_.notify_all(); // 通知所有在该条件上等待的线程
    }
}

void data_processing_thread()
{
    while (true)
    {
        std::unique_lock<std::mutex> unique_lock(mutex_);
        condition_.wait(unique_lock, []{return !queue_.empty();}); // 如果不满足条件会等待并释放锁
        data front = queue_.front(); // 如果满足条件，那会在拥有锁的前提下操作队列
        queue_.pop();
        unique_lock.unlock();
    }
}