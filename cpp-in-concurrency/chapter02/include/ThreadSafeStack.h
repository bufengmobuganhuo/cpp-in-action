//
// Created by zhangyu on 2026/10/8.
//

#pragma once
#include <exception>
#include <memory>
#include <mutex>
#include <stack>

struct empty_stack: std::exception
{
    const char* what() const noexcept override;
};

template<typename T>
class ThreadSafeStack
{
public:
    ThreadSafeStack();
    ThreadSafeStack(const ThreadSafeStack&);
    ThreadSafeStack& operator=(const ThreadSafeStack&) = delete;

    void push(T new_value);
    std::shared_ptr<T> pop();
    void pop(T& value);
    bool empty() const;
private:
    std::stack<T> inner_stack_;
    mutable std::mutex mutex_;
};
