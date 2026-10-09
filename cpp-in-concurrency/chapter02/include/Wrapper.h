//
// Created by zhangyu on 2026/10/8.
//

#pragma once
#include <mutex>

#include "Wrapper.h"

struct  SomeBigObj{};
class Wrapper
{
private:
    SomeBigObj obj_detail;
    std::mutex mutex_;
public:
    Wrapper(){};
    Wrapper(const SomeBigObj& sb): obj_detail(sb) {}

    friend void swap_1(Wrapper& lhs, Wrapper& rhs) noexcept //C++11的写法。这里使用friend的作用是标记这个方法不是Wrapper的成员函数
                                                            // 不需要通过this.swap()的方式来调用。调用方可以直接使用swap_1()
    {
        if (&lhs == &rhs)
        {
            return;
        }
        std::lock(lhs.mutex_, rhs.mutex_); // 支持锁住2个互斥锁
        std::lock_guard<std::mutex> lock_a(lhs.mutex_, std::adopt_lock); // 告知lock_guard互斥锁已经上锁，结果所有权即可
                                                                            // 使用这个lock_guard的目的是自动实现释放锁
        std::lock_guard<std::mutex> lock_b(rhs.mutex_, std::adopt_lock);
        // do something
    }

    friend void swap_2(Wrapper& lhs, Wrapper& rhs) noexcept //C++17的写法
    {
        if (&lhs == &rhs)
        {
            return;
        }
        std::scoped_lock<std::mutex, std::mutex> guard(lhs.mutex_, rhs.mutex_); //内部自动调用 std::lock 对多个mutex上锁，并且RAII自动解锁
        // do something
    }
};

