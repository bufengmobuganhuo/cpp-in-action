//
// Created by yuzhang on 2026/8/26.
//

#ifndef REACTOR_SERVER_TCPSERVER_H
#define REACTOR_SERVER_TCPSERVER_H
#include <functional>
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
    std::function<void(Socket* client_socket)> new_connection_callback_func_;
    std::function<void(Connection* conn,  std::string& message)> handle_message_callback_func_;
    std::function<void(Connection* conn)> on_write_complete_callback_func_;
    std::function<void(Connection* conn)> on_disconnect_callback_func_;
    std::function<void(Connection* conn)> on_error_callback_func_;
    std::function<void(EventLoop* loop)> on_timeout_callback_func_;
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

    void set_new_connection_callback_func_(std::function<void(Socket* client_socket)> fn);
    void set_handle_message_callback_func_(std::function<void(Connection* conn,  std::string& message)> fn);
    void set_on_write_complete_callback_func_(std::function<void(Connection* conn)> fn);
    void set_on_disconnect_callback_func_(std::function<void(Connection* conn)> fn);
    void set_on_error_callback_func_(std::function<void(Connection* conn)> fn);
    void set_on_timeout_callback_func_(std::function<void(EventLoop* loop)> fn);

};

#endif //REACTOR_SERVER_TCPSERVER_H
