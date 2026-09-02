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
    void handle_message(Connection* conn,  std::string message); // 处理接收到的报文，由connection回调
    void on_write_complete(Connection* conn); // 数据发完后的回调，由connection回调
    void on_disconnect(Connection* conn);
    void on_error(Connection* conn);
    void on_timeout(EventLoop* loop); // epoll_wait()的回调
};

#endif //REACTOR_SERVER_TCPSERVER_H
