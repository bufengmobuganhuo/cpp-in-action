//
// Created by yuzhang on 2026/8/30.
//

#include "include/Buffer.h"

Buffer::Buffer()
{

}

Buffer::~Buffer()
{

}

void Buffer::append(const char* data, size_t size)
{
    buf_.append(data, size);
}

void Buffer::erase(size_t start, size_t len)
{
    buf_.erase(start, len);
}

size_t Buffer::size()
{
    return buf_.size();
}

const char* Buffer::data() const
{
    return buf_.data();
}

void Buffer::clear()
{
    buf_.clear();
}
