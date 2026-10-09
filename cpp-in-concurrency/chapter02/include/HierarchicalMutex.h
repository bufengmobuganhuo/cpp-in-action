//
// Created by zhangyu on 2026/10/8.
//

#pragma once
#include <mutex>


class HierarchicalMutex
{
public:
    explicit HierarchicalMutex(unsigned long hierarchy) : hierarchy_(hierarchy), cur_locked_thread_hierarchy_(0) {}
    void lock();
    void unlock();
    bool try_lock();
private:
    std::mutex internal_mutex_;
    unsigned long const hierarchy_; // 锁支持锁定的最低层级，初始化时确定
    unsigned long cur_locked_thread_hierarchy_; // 如果 >0表示当前有线程获取了锁
    static thread_local unsigned long thread_hierarchy_; // 每个线程独享，表示当前线程已获取的最高层级的锁
                                                                    // 一开始默认为最大值，因为一开始所有的线程都可以加锁

    void check_for_hierarchy_violation() const; // 检查是否出现层级冲突，是否可以加锁
    void update_hierarchy(); // 获取锁后需要更新上述几个hierarchy的值
};
