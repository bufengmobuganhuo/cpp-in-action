#include <atomic>
//
// Created by zhangyu on 2026/10/16.
//
class SpinLockMutex
{
    std::atomic_flag flag_;
public:
    SpinLockMutex() : flag_(ATOMIC_FLAG_INIT)
    {

    }
    void lock()
    {
        while (flag_.test_and_set(std::memory_order_acquire));
    }

    void unlock()
    {
        flag_.clear(std::memory_order_release);
    }
};