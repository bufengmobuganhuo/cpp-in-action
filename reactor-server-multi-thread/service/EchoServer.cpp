//
// Created by yuzhang on 2026/9/2.
//

#include "EchoServer.h"

#include <iostream>
#include <unistd.h>
#include <syscall.h>

#include "../include/Socket.h"
#include "../include/Connection.h"
#include "../include/TcpServer.h"
#include "../include/ThreadPool.h"

namespace service
{
    EchoServer::EchoServer(const std::string& ip, const uint16_t port, int acceptor_thread_num, int worker_thread_num):
    tcp_server_(new TcpServer(ip, port, acceptor_thread_num))
    {
        tcp_server_->set_new_connection_callback_func_(std::bind(&EchoServer::handle_new_connection, this, std::placeholders::_1));
        tcp_server_->set_handle_message_callback_func_(std::bind(&EchoServer::handle_message, this, std::placeholders::_1, std::placeholders::_2));
        tcp_server_->set_on_write_complete_callback_func_(std::bind(&EchoServer::handle_write_complete, this, std::placeholders::_1));
        tcp_server_->set_on_disconnect_callback_func_(std::bind(&EchoServer::handle_disconnect, this, std::placeholders::_1));
        tcp_server_->set_on_error_callback_func_(std::bind(&EchoServer::handle_error, this, std::placeholders::_1));
        tcp_server_->set_on_timeout_callback_func_(std::bind(&EchoServer::handle_timeout, this, std::placeholders::_1));
        thread_pool_ = new ThreadPool(worker_thread_num, "worker");
    }

    EchoServer::~EchoServer()
    {
    }

    void EchoServer::start()
    {
        tcp_server_->start();
    }

    void EchoServer::handle_new_connection(std::shared_ptr<Connection> conn)
    {
        std::cout << "[EchoServer] New Connection..., thread is " << syscall(SYS_gettid) << std::endl;
    }

    void EchoServer::handle_message(const std::shared_ptr<Connection>& conn, std::string& message)
    {
        printf("[EchoServer] handle_message thread is %ld\n", syscall(SYS_gettid));
        thread_pool_->add_task([conn, message]
        {
            std::string replay = "reply: " + message;
            conn->send(replay.data(), replay.size());
        });
    }

    void EchoServer::handle_write_complete(const std::shared_ptr<Connection>& conn)
    {
        std::cout << "[EchoServer] Write Complete..." << std::endl;
    }

    void EchoServer::handle_disconnect(const std::shared_ptr<Connection>& conn)
    {
        std::cout << "[EchoServer] Disconnect..." << std::endl;
    }

    void EchoServer::handle_error(const std::shared_ptr<Connection>& conn)
    {
        std::cout << "[EchoServer] Error..." << std::endl;
    }

    void EchoServer::handle_timeout(EventLoop* loop)
    {
        std::cout << "[EchoServer] Timeout..." << std::endl;
    }
} // service