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
#include <memory>
#include "include/Buffer.h"


Connection::Connection(const std::unique_ptr<EventLoop>& event_loop, std::unique_ptr<Socket> socket) :
event_loop_(event_loop), client_socket_(std::move(socket)), disconnected_(false),
client_channel_(new Channel(event_loop_, client_socket_->fd()))
{
    // 绑定处理读事件的回调函数
    client_channel_->set_read_callback(std::bind(&Connection::on_message, this));
    client_channel_->set_write_callback(std::bind(&Connection::write, this));
    client_channel_->use_et();
    client_channel_->enable_reading();
    client_channel_->set_close_callback(std::bind(&Connection::on_disconnect, this));
    client_channel_->set_error_callback(std::bind(&Connection::on_error, this));
    input_buffer_ = new Buffer();
    output_buffer_ = new Buffer();
}

Connection::~Connection()
{
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
            client_channel_->remove_channel();
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
                on_message_callback_func_(shared_from_this(), message);
            }
            break;
        }
    }
}

void Connection::set_on_message_callback_func_(std::function<void(std::shared_ptr<Connection>, std::string&)> on_message_callback_func)
{
    on_message_callback_func_ = std::move(on_message_callback_func);
}

void Connection::set_on_write_complete_callback_func_(std::function<void(std::shared_ptr<Connection>)> on_write_complete_callback_func)
{
    on_write_complete_callback_func_ = std::move(on_write_complete_callback_func);
}

void Connection::on_disconnect()
{
    disconnected_ = true;
    on_disconnect_callback_func_(shared_from_this());
}

void Connection::on_error()
{
    on_disconnect_callback_func_(shared_from_this());
}

void Connection::set_on_disconnect_callback_func(std::function<void(std::shared_ptr<Connection>)> on_disconnect_callback_func)
{
    on_disconnect_callback_func_ = std::move(on_disconnect_callback_func);
}

void Connection::set_on_error_callback_func(std::function<void(std::shared_ptr<Connection>)> on_error_func)
{
    on_error_callback_func_ = std::move(on_error_func);
}

void Connection::send(const char* data, size_t size)
{
    if (disconnected_)
    {
        return;
    }
    output_buffer_->append_with_head(data, size);
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
    on_write_complete_callback_func_(shared_from_this());
}
