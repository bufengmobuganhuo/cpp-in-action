//
// Created by yuzhang on 2026/8/30.
//

#ifndef REACTOR_SERVER_BUFFER_H
#define REACTOR_SERVER_BUFFER_H
#include <string>

class Buffer
{
private:
    std::string buf_; // 存放数据的buffer
public:
    Buffer();
    ~Buffer();

    void append(const char* data, size_t size); // 追加到buffer中
    void erase(size_t start, size_t len); // 从指定位置开始删除len个字节
    size_t size(); // 返回buffer大小
    const char* data() const; // 返回buffer内的数据
    void clear(); // 清空buffer
};


#endif //REACTOR_SERVER_BUFFER_H
