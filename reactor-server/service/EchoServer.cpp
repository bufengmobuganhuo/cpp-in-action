//
// Created by yuzhang on 2026/9/2.
//

#include "EchoServer.h"

#include <iostream>

#include "../include/Socket.h"
#include "../include/Connection.h"
#include "../include/TcpServer.h"

namespace service
{
    EchoServer::EchoServer(const std::string& ip, const uint16_t port): tcp_server_(new TcpServer(ip, port))
    {
        tcp_server_->set_new_connection_callback_func_(std::bind(&EchoServer::handle_new_connection, this, std::placeholders::_1));
        tcp_server_->set_handle_message_callback_func_(std::bind(&EchoServer::handle_message, this, std::placeholders::_1, std::placeholders::_2));
        tcp_server_->set_on_write_complete_callback_func_(std::bind(&EchoServer::handle_write_complete, this, std::placeholders::_1));
        tcp_server_->set_on_disconnect_callback_func_(std::bind(&EchoServer::handle_disconnect, this, std::placeholders::_1));
        tcp_server_->set_on_error_callback_func_(std::bind(&EchoServer::handle_error, this, std::placeholders::_1));
        tcp_server_->set_on_timeout_callback_func_(std::bind(&EchoServer::handle_timeout, this, std::placeholders::_1));
    }

    EchoServer::~EchoServer()
    {
        delete tcp_server_;
    }

    void EchoServer::start()
    {
        tcp_server_->start();
    }

    void EchoServer::handle_new_connection(Socket* client_socket)
    {
        std::cout << "[EchoServer] New Connection..." << std::endl;
    }

    void EchoServer::handle_message(Connection* conn, std::string& message)
    {
        message = "reply: " + message;
        conn->send(message.data(), message.size());
    }

    void EchoServer::handle_write_complete(Connection* conn)
    {
        std::cout << "[EchoServer] Write Complete..." << std::endl;
    }

    void EchoServer::handle_disconnect(Connection* conn)
    {
        std::cout << "[EchoServer] Disconnect..." << std::endl;
    }

    void EchoServer::handle_error(Connection* conn)
    {
        std::cout << "[EchoServer] Error..." << std::endl;
    }

    void EchoServer::handle_timeout(EventLoop* loop)
    {
        std::cout << "[EchoServer] Timeout..." << std::endl;
    }
} // service