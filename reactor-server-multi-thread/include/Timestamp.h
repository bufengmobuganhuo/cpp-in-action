//
// Created by yuzhang on 2026/9/9.
//

#ifndef REACTOR_SERVER_TIMESTAMP_H
#define REACTOR_SERVER_TIMESTAMP_H
#include <cstdint>
#include <ctime>
#include <string>

class Timestamp
{
private:
    time_t sec_since_epoch_; // 1970到现在的秒数
public:
    Timestamp();
    Timestamp(int64_t sec_since_epoch);
    ~Timestamp();

    static Timestamp now();

    time_t to_int() const; // 返回整数表示的时间
    std::string to_string() const; // 返回 yyyy-MM-dd hh:mm:ss
};

#endif //REACTOR_SERVER_TIMESTAMP_H
