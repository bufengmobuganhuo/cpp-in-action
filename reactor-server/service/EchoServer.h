//
// Created by yuzhang on 2026/9/2.
// 支持实时回显的服务器

#ifndef REACTOR_SERVER_ECHOSERVER_H
#define REACTOR_SERVER_ECHOSERVER_H
#include <string>

class TcpServer;
class Socket;
class Connection;
class EventLoop;

namespace service
{


    class EchoServer
    {
    private:
        TcpServer* tcp_server_;
    public:
        EchoServer(const std::string& ip, const uint16_t port);
        ~EchoServer();

        void start();
        void handle_new_connection(Socket* client_socket);
        void handle_message(Connection* conn,  std::string& message);
        void handle_write_complete(Connection* conn);
        void handle_disconnect(Connection* conn);
        void handle_error(Connection* conn);
        void handle_timeout(EventLoop* loop);
    };
} // service

#endif //REACTOR_SERVER_ECHOSERVER_H
