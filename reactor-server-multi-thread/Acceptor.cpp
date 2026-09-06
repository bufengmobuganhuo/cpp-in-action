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

Acceptor::Acceptor(EventLoop* event_loop, const std::string& ip, uint16_t port): event_loop_(event_loop)
{
    // 创建服务端用于监听的server_socket_fd
    serv_socket_ = new Socket(create_non_blocking());
    // 设置fd的属性
    serv_socket_->set_reuse_addr(true);
    serv_socket_->set_reuse_port(true);
    serv_socket_->set_tcp_no_delay(true);
    serv_socket_->set_keep_alive(true);

    InetAddress serv_addr(ip, port);

    serv_socket_->bind(serv_addr);
    serv_socket_->listen();

    std::cout << "[Server] Listening on " << ip << ":" << port << std::endl;

    accept_channel_ = new Channel(event_loop_, serv_socket_->fd());
    // 绑定处理读事件的回调函数
    accept_channel_->set_read_callback(std::bind(&Acceptor::new_connection, this));
    accept_channel_->enable_reading();
}

Acceptor::~Acceptor()
{
    delete serv_socket_;
    delete accept_channel_;
}

void Acceptor::new_connection() const
{
    InetAddress client_addr;
    auto* client_socket = new Socket(serv_socket_->accept(client_addr), client_addr.ip(), client_addr.port());
    client_socket->set_addr(client_addr.ip(), client_addr.port());
    new_connection_func_(client_socket);

}

void Acceptor::set_new_connection_func(std::function<void(Socket*)> fn)
{
    new_connection_func_ = fn;
}
