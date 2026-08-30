//
// Created by yuzhang on 2026/8/30.
//
#include "include/Connection.h"
#include "include/Channel.h"
#include <functional>
#include "include/Socket.h"
#include <iostream>
#include <utility>

Connection::Connection(EventLoop* event_loop, Socket* socket) : event_loop_(event_loop), client_socket_(socket)
{
    Channel* client_channel = new Channel(event_loop_, socket->fd());
    // 绑定处理读事件的回调函数
    client_channel->set_read_callback(std::bind(&Channel::on_message, client_channel));
    client_channel->use_et();
    client_channel->enable_reading();
    client_channel->set_close_callback(std::bind(&Connection::on_disconnect, this));
    client_channel->set_error_callback(std::bind(&Connection::on_error, this));
    client_channel_ = client_channel;
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

void Connection::on_disconnect()
{
    on_disconnect_func_(this);
}

void Connection::on_error()
{
    on_disconnect_func_(this);
}

void Connection::set_on_disconnect_func(std::function<void(Connection*)> on_close_func)
{
    on_disconnect_func_ = std::move(on_close_func);
}

void Connection::set_on_error_func(std::function<void(Connection*)> on_error_func)
{
    on_error_func_ = std::move(on_error_func);
}
