//
// Created by yuzhang on 2026/8/30.
//

#include "include/Buffer.h"

#include <cstring>

Buffer::Buffer(uint64_t sep) : sep_(sep)
{
}

Buffer::~Buffer()
{
}

void Buffer::append(const char* data, size_t size)
{
    buf_.append(data, size);
}

void Buffer::append_with_head(const char* data, size_t size)
{
    if (sep_ == 1)
    {
        buf_.append((char*)&size, 4);
    }
    else
    {
        buf_.append("\r\n\r\n");
    }
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

bool Buffer::pick_message(std::string& target)
{
    if (buf_.size() == 0)
    {
        return false;
    }
    if (sep_ == 0)
    {
        target = buf_;
        buf_.clear();
    }
    else if (sep_ == 1)
    {
        int msg_len;
        memcpy(&msg_len, buf_.data(), 4);
        if (buf_.size() < msg_len + 4)
        {
            return false;
        }
        target = buf_.substr(4, msg_len);
        buf_.erase(0, msg_len + 4);
    }
    else
    {
        size_t header_len = strlen("\r\n\r\n");
        target = buf_.substr(header_len);
        buf_.clear();
    }
    return true;
}
