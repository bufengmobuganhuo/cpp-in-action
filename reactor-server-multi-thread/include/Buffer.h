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
    const uint64_t sep_; // 报文分隔符：0-无分隔符（视频会议），1-四字节的报文头，2-\r\n\r\n分隔符（http协议）
public:
    Buffer(uint64_t sep=1);
    ~Buffer();

    void append(const char* data, size_t size); // 追加到buffer中
    void append_with_head(const char* data, size_t size); // 附加报文头部后放入buffer
    void erase(size_t start, size_t len); // 从指定位置开始删除len个字节
    size_t size(); // 返回buffer大小
    const char* data() const; // 返回buffer内的数据
    void clear(); // 清空buffer
    bool pick_message(std::string& target);
};


#endif //REACTOR_SERVER_BUFFER_H
