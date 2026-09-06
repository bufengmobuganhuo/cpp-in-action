//
// Created by yuzhang on 2026/8/26.
//

#ifndef REACTOR_SERVER_TCPSERVER_H
#define REACTOR_SERVER_TCPSERVER_H
#include <functional>
#include <string>
#include <map>
#include <vector>
#include <memory>

#include "Connection.h"

class EventLoop;
class Socket;
class Channel;
class Acceptor;
class Connection;
class ThreadPool;

class TcpServer
{
private:
    std::unique_ptr<EventLoop> main_event_loop_; // 主事件循环
    std::vector<std::unique_ptr<EventLoop>> sub_event_loops_; // 从事件循环
    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<ThreadPool> thread_pool_; // 线程池
    int thread_num_; // 线程池大小
    std::map<int, std::shared_ptr<Connection>> conns_; // <fd, connection>
    std::function<void(std::shared_ptr<Connection> conn)> new_connection_callback_func_;
    std::function<void(std::shared_ptr<Connection> conn, std::string& message)> handle_message_callback_func_;
    std::function<void(std::shared_ptr<Connection> conn)> on_write_complete_callback_func_;
    std::function<void(std::shared_ptr<Connection> conn)> on_disconnect_callback_func_;
    std::function<void(std::shared_ptr<Connection> conn)> on_error_callback_func_;
    std::function<void(EventLoop* loop)> on_timeout_callback_func_;

public:
    TcpServer(const std::string& ip, uint16_t port, int thread_num = 3);
    ~TcpServer();
    void start(); // 开启服务器
    void new_connection(std::unique_ptr<Socket> client_socket); // 创建Connection
    void handle_message(std::shared_ptr<Connection> conn, std::string message); // 处理接收到的报文，由connection回调
    void on_write_complete(std::shared_ptr<Connection> conn); // 数据发完后的回调，由connection回调
    void on_disconnect(const std::shared_ptr<Connection>& conn);
    void on_error(const std::shared_ptr<Connection>& conn);
    void on_timeout(EventLoop* loop); // epoll_wait()的回调

    void set_new_connection_callback_func_(std::function<void(std::shared_ptr<Connection> conn)> fn);
    void set_handle_message_callback_func_(std::function<void(std::shared_ptr<Connection> conn, std::string& message)> fn);
    void set_on_write_complete_callback_func_(std::function<void(std::shared_ptr<Connection> conn)> fn);
    void set_on_disconnect_callback_func_(std::function<void(std::shared_ptr<Connection> conn)> fn);
    void set_on_error_callback_func_(std::function<void(std::shared_ptr<Connection> conn)> fn);
    void set_on_timeout_callback_func_(std::function<void(EventLoop* loop)> fn);
};

#endif //REACTOR_SERVER_TCPSERVER_H
