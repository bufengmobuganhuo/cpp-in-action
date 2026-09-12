//
// Created by yuzhang on 2026/9/2.
// 支持实时回显的服务器

#ifndef REACTOR_SERVER_ECHOSERVER_H
#define REACTOR_SERVER_ECHOSERVER_H
#include <string>
#include <memory>

class TcpServer;
class Socket;
class Connection;
class EventLoop;
class ThreadPool;

namespace service
{
    class EchoServer
    {
    private:
        std::unique_ptr<TcpServer> tcp_server_;
        ThreadPool* thread_pool_; // 工作线程池
    public:
        EchoServer(const std::string& ip, uint16_t port, int acceptor_thread_num = 3, int worker_thread_num_ = 3);
        ~EchoServer();

        void start();
        void stop();
        void handle_new_connection(std::shared_ptr<Connection> conn);
        void handle_message(const std::shared_ptr<Connection>& conn, std::string& message);
        void handle_write_complete(const std::shared_ptr<Connection>& conn);
        void handle_disconnect(const std::shared_ptr<Connection>& conn);
        void handle_error(const std::shared_ptr<Connection>& conn);
        void handle_timeout(EventLoop* loop);
    };
} // service

#endif //REACTOR_SERVER_ECHOSERVER_H
