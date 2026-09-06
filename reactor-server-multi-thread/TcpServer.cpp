//
// Created by yuzhang on 2026/8/26.
//
#include "include/TcpServer.h"

#include <iostream>
#include <utility>

#include "include/Acceptor.h"
#include "include/Connection.h"
#include "include/InetAddress.h"
#include "include/EventLoop.h"
#include "include/ThreadPool.h"

TcpServer::TcpServer(const std::string& ip, uint16_t port, int thread_num)
{
    main_event_loop_ = new EventLoop();
    main_event_loop_->set_on_timeout_callback_func_(std::bind(&TcpServer::on_timeout, this, std::placeholders::_1));

    acceptor_ = new Acceptor(main_event_loop_, ip, port);
    acceptor_->set_new_connection_func(std::bind(&TcpServer::new_connection, this, std::placeholders::_1));

    thread_num_ = thread_num;
    thread_pool_ = new ThreadPool(thread_num, "acceptor");

    for (int i = 0; i < thread_num; i++)
    {
        EventLoop* event_loop = new EventLoop();
        sub_event_loops_.push_back(event_loop); // 创建从事件循环
        event_loop->set_on_timeout_callback_func_(std::bind(&TcpServer::on_timeout, this, std::placeholders::_1));
        thread_pool_->add_task(std::bind(&EventLoop::run, event_loop));
    }
}

TcpServer::~TcpServer()
{
    delete main_event_loop_;
    delete acceptor_;
    for (auto conn : conns_)
    {
        delete conn.second;
    }
}

void TcpServer::start()
{
    std::cout << "[Server] Server is running, waiting for connections..." << std::endl;
    main_event_loop_->run();
}

void TcpServer::new_connection(Socket* client_socket)
{
    Connection* connection = new Connection(sub_event_loops_[client_socket->fd() % thread_num_], client_socket);
    connection->set_on_message_callback_func_(std::bind(&TcpServer::handle_message, this, std::placeholders::_1, std::placeholders::_2));
    connection->set_on_write_complete_callback_func_(std::bind(&TcpServer::on_write_complete, this, std::placeholders::_1));
    connection->set_on_disconnect_callback_func(std::bind(&TcpServer::on_disconnect, this, std::placeholders::_1));
    connection->set_on_error_callback_func(std::bind(&TcpServer::on_error, this, std::placeholders::_1));
    printf("new client(fd=%d,ip=%s,port=%d) ok.\n", client_socket->fd(), connection->ip().c_str(),
           connection->port());
    conns_[connection->fd()] = connection;
    if (new_connection_callback_func_)
    {
        new_connection_callback_func_(client_socket);
    }
}

void TcpServer::handle_message(Connection* conn, std::string message)
{
    if (handle_message_callback_func_)
    {
        handle_message_callback_func_(conn, message);
    }
}

void TcpServer::on_write_complete(Connection* conn)
{
    std::cout << "[EpollServer] write complete" << std::endl;
    if (on_write_complete_callback_func_)
    {
        on_write_complete_callback_func_(conn);
    }
}

void TcpServer::on_disconnect(Connection* conn)
{
    printf("[EpollServer] event_fd=%d is closed", conn->fd());
    conns_.erase(conn->fd());
    if (on_disconnect_callback_func_)
    {
        on_disconnect_callback_func_(conn);
    }
    delete conn;
}

void TcpServer::on_error(Connection* conn)
{
    std::cout << "[EpollServer] error " << "for client: " << conn->fd() << std::endl;
    conns_.erase(conn->fd());
    if (on_error_callback_func_)
    {
        on_error_callback_func_(conn);
    }
    delete conn;
}

void TcpServer::on_timeout(EventLoop* loop)
{
    std::cout << "[EpollServer] timeout " << std::endl;
    if (on_timeout_callback_func_)
    {
        on_timeout_callback_func_(loop);
    }
}

void TcpServer::set_new_connection_callback_func_(std::function<void(Socket* client_socket)> fn)
{
    new_connection_callback_func_ = std::move(fn);
}

void TcpServer::set_handle_message_callback_func_(std::function<void(Connection* conn, std::string& message)> fn)
{
    handle_message_callback_func_ = std::move(fn);
}

void TcpServer::set_on_write_complete_callback_func_(std::function<void(Connection* conn)> fn)
{
    on_write_complete_callback_func_ = std::move(fn);
}

void TcpServer::set_on_disconnect_callback_func_(std::function<void(Connection* conn)> fn)
{
    on_disconnect_callback_func_ = std::move(fn);
}

void TcpServer::set_on_error_callback_func_(std::function<void(Connection* conn)> fn)
{
    on_error_callback_func_ = std::move(fn);
}

void TcpServer::set_on_timeout_callback_func_(std::function<void(EventLoop* loop)> fn)
{
    on_timeout_callback_func_ = std::move(fn);
}
