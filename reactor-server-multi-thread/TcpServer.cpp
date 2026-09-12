//
// Created by yuzhang on 2026/8/26.
//
#include "include/TcpServer.h"

#include <iostream>
#include <memory>
#include <utility>

#include "include/Acceptor.h"
#include "include/Connection.h"
#include "include/InetAddress.h"
#include "include/EventLoop.h"
#include "include/ThreadPool.h"

TcpServer::TcpServer(const std::string& ip, uint16_t port, int thread_num) :
main_event_loop_(new EventLoop(true)), acceptor_(new Acceptor(main_event_loop_.get(), ip, port)),
thread_pool_(new ThreadPool(thread_num, "acceptor"))
{
    main_event_loop_->set_on_timeout_callback_func_(std::bind(&TcpServer::on_timeout, this, std::placeholders::_1));
    acceptor_->set_new_connection_func(std::bind(&TcpServer::new_connection, this, std::placeholders::_1));
    thread_num_ = thread_num;
    for (int i = 0; i < thread_num; i++)
    {
        sub_event_loops_.emplace_back(new EventLoop(false)); // 创建从事件循环
        sub_event_loops_[i]->set_on_timeout_callback_func_(std::bind(&TcpServer::on_timeout, this, std::placeholders::_1));
        sub_event_loops_[i]->set_on_remove_conn_callback_func_(std::bind(&TcpServer::remove_conn, this, std::placeholders::_1));
        thread_pool_->add_task(std::bind(&EventLoop::run, sub_event_loops_[i].get()));
    }
}

TcpServer::~TcpServer()
= default;

void TcpServer::start()
{
    std::cout << "[Server] Server is running, waiting for connections..." << std::endl;
    main_event_loop_->run();
}

void TcpServer::stop()
{
    main_event_loop_->stop();
    for (int i = 0; i < sub_event_loops_.size(); i++)
    {
        sub_event_loops_[i]->stop();
    }
    thread_pool_->shutdown();
}

void TcpServer::new_connection(std::unique_ptr<Socket> client_socket)
{
    int fd = client_socket->fd();
    std::shared_ptr<Connection> connection = std::make_shared<Connection>(sub_event_loops_[fd % thread_num_].get(), std::move(client_socket));
    connection->set_on_message_callback_func_(std::bind(&TcpServer::handle_message, this, std::placeholders::_1, std::placeholders::_2));
    connection->set_on_write_complete_callback_func_(std::bind(&TcpServer::on_write_complete, this, std::placeholders::_1));
    connection->set_on_disconnect_callback_func(std::bind(&TcpServer::on_disconnect, this, std::placeholders::_1));
    connection->set_on_error_callback_func(std::bind(&TcpServer::on_error, this, std::placeholders::_1));
    printf("new client(fd=%d,ip=%s,port=%d) ok.\n", fd, connection->ip().c_str(),
           connection->port());
    {
        std::lock_guard<std::mutex> gd(conns_mutex_);
        conns_[connection->fd()] = connection;  // 因为主事件循环和从事件循环都会操作这个map，因此需要加锁
    }

    sub_event_loops_[fd % thread_num_].get()->new_connection(connection);
    if (new_connection_callback_func_)
    {
        new_connection_callback_func_(connection);
    }
}

void TcpServer::handle_message(std::shared_ptr<Connection> conn, std::string message)
{
    if (handle_message_callback_func_)
    {
        handle_message_callback_func_(conn, message);
    }
}

void TcpServer::on_write_complete(std::shared_ptr<Connection> conn)
{
    std::cout << "[EpollServer] write complete" << std::endl;
    if (on_write_complete_callback_func_)
    {
        on_write_complete_callback_func_(conn);
    }
}

void TcpServer::on_disconnect(const std::shared_ptr<Connection>& conn)
{
    printf("[EpollServer] event_fd=%d is closed", conn->fd());
    {
        std::lock_guard<std::mutex> gd(conns_mutex_);
        conns_.erase(conn->fd());
    }
    if (on_disconnect_callback_func_)
    {
        on_disconnect_callback_func_(conn);
    }
}

void TcpServer::on_error(const std::shared_ptr<Connection>& conn)
{
    std::cout << "[EpollServer] error " << "for client: " << conn->fd() << std::endl;
    {
        std::lock_guard<std::mutex> gd(conns_mutex_);
        conns_.erase(conn->fd());
    }
    if (on_error_callback_func_)
    {
        on_error_callback_func_(conn);
    }
}

void TcpServer::on_timeout(EventLoop* loop)
{
    std::cout << "[EpollServer] timeout " << std::endl;
    if (on_timeout_callback_func_)
    {
        on_timeout_callback_func_(loop);
    }
}

void TcpServer::remove_conn(int fd)
{
    std::lock_guard<std::mutex> gd(conns_mutex_);
    conns_.erase(fd);
}

void TcpServer::set_new_connection_callback_func_(std::function<void(std::shared_ptr<Connection> conn)> fn)
{
    new_connection_callback_func_ = std::move(fn);
}

void TcpServer::set_handle_message_callback_func_(std::function<void(std::shared_ptr<Connection> conn, std::string& message)> fn)
{
    handle_message_callback_func_ = std::move(fn);
}

void TcpServer::set_on_write_complete_callback_func_(std::function<void(std::shared_ptr<Connection> conn)> fn)
{
    on_write_complete_callback_func_ = std::move(fn);
}

void TcpServer::set_on_disconnect_callback_func_(std::function<void(std::shared_ptr<Connection> conn)> fn)
{
    on_disconnect_callback_func_ = std::move(fn);
}

void TcpServer::set_on_error_callback_func_(std::function<void(std::shared_ptr<Connection> conn)> fn)
{
    on_error_callback_func_ = std::move(fn);
}

void TcpServer::set_on_timeout_callback_func_(std::function<void(EventLoop* loop)> fn)
{
    on_timeout_callback_func_ = std::move(fn);
}
