//
// Created by yuzhang on 2026/8/30.
//
#include "include/Connection.h"
#include "include/Channel.h"
#include <functional>
#include "include/EventLoop.h"
#include "include/Socket.h"
#include <iostream>
#include <unistd.h>

Connection::Connection(EventLoop* eventLoop, Socket* socket) : eventLoop_(eventLoop), clientSocket_(socket)
{
    Channel* client_channel = new Channel(eventLoop_, socket->fd());
    // 绑定处理读事件的回调函数
    client_channel->setReadCallback(std::bind(&Channel::onMessage, client_channel));
    client_channel->useET();
    client_channel->enableReading();
    clientChannel_ = client_channel;
}

Connection::~Connection()
{
    delete clientSocket_;
    delete clientChannel_;
}

int Connection::fd() const
{
    return clientSocket_->fd();
}

std::string Connection::ip() const
{
    return clientSocket_->ip();
}

uint16_t Connection::port() const
{
    return clientSocket_->port();
}

void Connection::on_close() const
{
    printf("[Server] event_fd=%d is closed", fd());
    close(fd());
}

void Connection::on_error(uint32_t unknownEvents) const
{
    std::cout << "[Server] unknow event: " << unknownEvents << "for client: " << fd() << std::endl;
    close(fd());
}
