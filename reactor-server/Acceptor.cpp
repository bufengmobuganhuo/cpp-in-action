//
// Created by yuzhang on 2026/8/27.
//
#include "include/Acceptor.h"

#include <functional>
#include <iostream>

#include "include/Channel.h"
#include "include/Connection.h"
#include "include/Epoll.h"
#include "include/EventLoop.h"
#include "include/InetAddress.h"
#include "include/Socket.h"

Acceptor::Acceptor(EventLoop* eventLoop_, const std::string& ip, uint16_t port): eventLoop_(eventLoop_)
{
    // 创建服务端用于监听的server_socket_fd
    servSocket_ = new Socket(createNonBlocking());
    // 设置fd的属性
    servSocket_->setReuseAddr(true);
    servSocket_->setReusePort(true);
    servSocket_->setTcpNoDelay(true);
    servSocket_->setKeepAlive(true);

    InetAddress serv_addr(ip, port);

    servSocket_->bind(serv_addr);
    servSocket_->listen();

    std::cout << "[Server] Listening on " << ip << ":" << port << std::endl;

    acceptChannel_ = new Channel(eventLoop_, servSocket_->fd());
    // 绑定处理读事件的回调函数
    acceptChannel_->setReadCallback(std::bind(&Acceptor::newConnection, this));
    acceptChannel_->enableReading();
}

Acceptor::~Acceptor()
{
    delete servSocket_;
    delete acceptChannel_;
}

void Acceptor::newConnection() const
{
    InetAddress client_addr;
    auto* client_socket = new Socket(servSocket_->accept(client_addr), client_addr.ip(), client_addr.port());
    newConnectionFunc_(client_socket);

}

void Acceptor::setNewConnectionFunc(std::function<void(Socket*)> fn)
{
    newConnectionFunc_ = fn;
}
