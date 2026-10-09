//
// Created by zhangyu on 2026/10/8.
//

#include "HierarchicalMutex.h"

#include <stdexcept>

void HierarchicalMutex::lock()
{
    check_for_hierarchy_violation();
    internal_mutex_.lock();
    update_hierarchy();
}

void HierarchicalMutex::unlock()
{
    if (thread_hierarchy_ != hierarchy_)
    {
        throw std::logic_error("mutex hierarchy violated"); // 锁只能从逐层从下往上释放
    }
    thread_hierarchy_ = cur_locked_thread_hierarchy_;
    internal_mutex_.unlock();
}

bool HierarchicalMutex::try_lock()
{
    check_for_hierarchy_violation();
    if (!internal_mutex_.try_lock())
    {
        return false;
    }
    update_hierarchy();
    return true;
}

void HierarchicalMutex::check_for_hierarchy_violation() const
{
    if (thread_hierarchy_ <= hierarchy_)
    {
        throw std::logic_error("mutex hierarchy violated"); // 只能逐层向下加锁
    }
}

void HierarchicalMutex::update_hierarchy()
{
    cur_locked_thread_hierarchy_ = thread_hierarchy_; // 保存当前已获取锁的线程的hierarchy
    thread_hierarchy_ = hierarchy_; // 更新当前线程可以加锁的最高层级号
}

thread_local unsigned long HierarchicalMutex::thread_hierarchy_(ULONG_MAX); // 一开始默认为最大值，因为一开始所有的线程都可以加锁