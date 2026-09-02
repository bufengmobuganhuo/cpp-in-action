//
// Created by yuzhang on 2026/8/30.
//
#include "include/Connection.h"

#include <cstring>

#include "include/Channel.h"
#include <functional>
#include "include/Socket.h"
#include <iostream>
#include <strings.h>
#include <unistd.h>
#include <utility>

#include "include/Buffer.h"

Connection::Connection(EventLoop* event_loop, Socket* socket) : event_loop_(event_loop), client_socket_(socket)
{
    Channel* client_channel = new Channel(event_loop_, socket->fd());
    // 绑定处理读事件的回调函数
    client_channel->set_read_callback(std::bind(&Connection::on_message, this));
    client_channel->set_write_callback(std::bind(&Connection::write, this));
    client_channel->use_et();
    client_channel->enable_reading();
    client_channel->set_close_callback(std::bind(&Connection::on_disconnect, this));
    client_channel->set_error_callback(std::bind(&Connection::on_error, this));
    client_channel_ = client_channel;
    input_buffer_ = new Buffer();
    output_buffer_ = new Buffer();
}

Connection::~Connection()
{
    delete client_socket_;
    delete client_channel_;
}

int Connection::fd() const
{
    return client_socket_->fd();
}

std::string Connection::ip() const
{
    return client_socket_->ip();
}

uint16_t Connection::port() const
{
    return client_socket_->port();
}

void Connection::on_message()
{
    // 客户端有数据可读，要一直读完为止
    // 处理客户端数据
    char buffer[k_buffer_size] = {0};
    while (true)
    {
        // 清理buffer
        bzero(&buffer, sizeof(buffer));
        int bytes_read = read(fd(), buffer, k_buffer_size - 1);
        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            // 先放入buffer
            input_buffer_->append(buffer, bytes_read);
        }
        else if (bytes_read == 0)
        {
            // 客户端连接已断开
            std::cout << "[Server] Client disconnected, fd: " << fd() << std::endl;
            on_disconnect();
            break;
        }
        else if (bytes_read == -1 && errno == EINTR)
        {
            // 读取数据的时候被信号中断，继续读取
            continue;
        }
        else if (bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            while (true)
            {
                int len;
                memcpy(&len, input_buffer_->data(), 4); // 从input_buffer中读取数据的长度
                if (input_buffer_->size() < len + 4) // 报文内容还未完全接收到
                {
                    break;
                }

                std::string message(input_buffer_->data() + 4, len);
                input_buffer_->erase(0, len + 4); // 清理input_buffer

                printf("[EpollServer] received message (event_fd=%d: %s)\n", fd(), message.c_str());

                // 处理消息
                on_message_callback_func_(this, message);
            }
            break;
        }
    }
}

void Connection::set_on_message_callback_func_(std::function<void(Connection*, std::string)> on_message_callback_func)
{
    on_message_callback_func_ = std::move(on_message_callback_func);
}

void Connection::set_on_write_complete_callback_func_(std::function<void(Connection*)> on_write_complete_callback_func)
{
    on_write_complete_callback_func_ = std::move(on_write_complete_callback_func);
}

void Connection::on_disconnect()
{
    on_disconnect_callback_func_(this);
}

void Connection::on_error()
{
    on_disconnect_callback_func_(this);
}

void Connection::set_on_disconnect_callback_func(std::function<void(Connection*)> on_disconnect_callback_func)
{
    on_disconnect_callback_func_ = std::move(on_disconnect_callback_func);
}

void Connection::set_on_error_callback_func(std::function<void(Connection*)> on_error_func)
{
    on_error_callback_func_ = std::move(on_error_func);
}

void Connection::send(const char* data, size_t size)
{
    output_buffer_->append(data, size);
    client_channel_->enable_writing(); // 注册写事件
}

void Connection::write()
{
    ssize_t writen_len = ::send(fd(), output_buffer_->data(), output_buffer_->size(), 0);
    if (writen_len > 0)
    {
        output_buffer_->erase(0, writen_len);
    }
    if (output_buffer_->size() == 0)
    {
        // 如果已经发完了，则不需要再关注写事件，以防一直死循环
        client_channel_->disable_writing();
    }
    on_write_complete_callback_func_(this);
}
