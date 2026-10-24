//
// Created by yuzhang on 2026/10/18.
//
#include <atomic>

std::atomic<int> data;
std::atomic<int> sync;

void thread_1()
{
    data.store(1, std::memory_order_relaxed); // 最“松弛”的顺序约束
    sync.store(1, std::memory_order_release);
}

void thread_2()
{
    int expected = 1;
    while (!sync.compare_exchange_strong(expected, 2, std::memory_order_acq_rel))
    {
        expected = 1; // compare_exchange_strong失败后expected = 原子变量的值，因此这里需要恢复expected的值
    };
}

void thread_3()
{
    while (sync.load(std::memory_order_acquire) < 2) {};
}