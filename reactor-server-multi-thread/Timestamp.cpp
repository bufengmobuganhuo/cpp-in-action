//
// Created by yuzhang on 2026/9/9.
//
#include "include/Timestamp.h"

Timestamp::Timestamp()
{
    sec_since_epoch_ = time(0);
}

Timestamp::Timestamp(int64_t sec_since_epoch)
    : sec_since_epoch_(sec_since_epoch)
{
}

Timestamp Timestamp::now()
{
    return {};
}

Timestamp::~Timestamp()
{
}

time_t Timestamp::to_int() const
{
    return sec_since_epoch_;
}

std::string Timestamp::to_string() const
{
    char buf[32] = {0};
    tm* tm_time = localtime(&sec_since_epoch_);
    snprintf(buf, 20, "%4d-%02d-%02d %02d:%02d:%02d",
             tm_time->tm_year + 1900,
             tm_time->tm_mon + 1,
             tm_time->tm_mday,
             tm_time->tm_hour,
             tm_time->tm_min,
             tm_time->tm_sec
    );
    return buf;
}
