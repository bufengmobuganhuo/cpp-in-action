//
// Created by zhangyu on 2026/10/8.
//
#include "include/ThreadSafeStack.h"

const char* empty_stack::what() const noexcept
{
    return "empty_stack: stack is empty";
}

template <typename T>
ThreadSafeStack<T>::ThreadSafeStack()
{
}

template <typename T>
ThreadSafeStack<T>::ThreadSafeStack(const ThreadSafeStack& other): inner_stack_(other.inner_stack_)
{
}

template <typename T>
void ThreadSafeStack<T>::push(T new_value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    inner_stack_.push(new_value);
}

template <typename T>
std::shared_ptr<T> ThreadSafeStack<T>::pop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (inner_stack_.empty())
    {
        throw empty_stack();
    }
    std::shared_ptr<T> const res = std::make_shared<T>(inner_stack_.top());
    inner_stack_.pop();
    return res;
}

template <typename T>
void ThreadSafeStack<T>::pop(T& value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (inner_stack_.empty())
    {
        throw empty_stack();
    }
    value = inner_stack_.top();
    inner_stack_.pop();
}

template <typename T>
bool ThreadSafeStack<T>::empty() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return inner_stack_.empty();
}