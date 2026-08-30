//
// Created by yuzhang on 2026/8/26.
//
#include "include/TcpServer.h"

#include <iostream>

#include "include/Acceptor.h"
#include "include/Channel.h"
#include "include/Connection.h"
#include "include/InetAddress.h"
#include "include/EventLoop.h"
#include <unistd.h>

TcpServer::TcpServer(const std::string& ip, uint16_t port)
{
    event_loop_ = new EventLoop();
    acceptor_ = new Acceptor(event_loop_, ip, port);
    acceptor_->set_new_connection_func(std::bind(&TcpServer::new_connection, this, std::placeholders::_1));
}

TcpServer::~TcpServer()
{
    delete event_loop_;
    delete acceptor_;
    for (auto conn : conns_)
    {
        delete conn.second;
    }
}

void TcpServer::start()
{
    std::cout << "[Server] Server is running, waiting for connections..." << std::endl;
    event_loop_->run();
}

void TcpServer::new_connection(Socket* client_socket)
{
    Connection* connection = new Connection(event_loop_, client_socket);
    connection->set_on_disconnect_func(std::bind(&TcpServer::on_disconnect, this, std::placeholders::_1));
    connection->set_on_error_func(std::bind(&TcpServer::on_error, this, std::placeholders::_1));
    printf("new client(fd=%d,ip=%s,port=%d) ok.\n", client_socket->fd(), connection->ip().c_str(),
           connection->port());
    conns_[connection->fd()] = connection;
}

void TcpServer::on_disconnect(Connection* conn)
{
    printf("[Server] event_fd=%d is closed", conn->fd());
    conns_.erase(conn->fd());
    delete conn;
}

void TcpServer::on_error(Connection* conn)
{
    std::cout << "[Server] error " << "for client: " << conn->fd() << std::endl;
    conns_.erase(conn->fd());
    delete conn;
}
