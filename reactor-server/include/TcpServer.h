//
// Created by yuzhang on 2026/8/26.
//

#ifndef REACTOR_SERVER_TCPSERVER_H
#define REACTOR_SERVER_TCPSERVER_H
#include <string>
#include <map>

class EventLoop;
class Socket;
class Channel;
class Acceptor;
class Connection;

class TcpServer
{
private:
    EventLoop* event_loop_;
    Acceptor* acceptor_;
    std::map<int, Connection*> conns_; // <fd, connection>
public:
    TcpServer(const std::string& ip, uint16_t port);
    ~TcpServer();
    void start(); // 开启服务器
    void new_connection(Socket* client_socket); // 创建Connection
    void on_disconnect(Connection* conn);
    void on_error(Connection* conn);
};

#endif //REACTOR_SERVER_TCPSERVER_H
