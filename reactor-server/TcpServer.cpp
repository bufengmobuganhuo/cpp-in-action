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

TcpServer::TcpServer(const std::string& ip, uint16_t port)
{
    eventLoop_ = new EventLoop();
    acceptor_ = new Acceptor(eventLoop_, ip, port);
    acceptor_->setNewConnectionFunc(std::bind(&TcpServer::newConnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer()
{
    delete eventLoop_;
    delete acceptor_;
    for (auto conn : conns_)
    {
        delete conn.second;
    }
}

void TcpServer::start()
{
    std::cout << "[Server] Server is running, waiting for connections..." << std::endl;
    eventLoop_->run();
}

void TcpServer::newConnection(Socket* client_socket)
{
    Connection* connection = new Connection(eventLoop_, client_socket);
    printf("new client(fd=%d,ip=%s,port=%d) ok.\n", client_socket->fd(), connection->ip().c_str(),
           connection->port());
    conns_[connection->fd()] = connection;
}

void TcpServer::onDisconnect(int fd)
{
    
}
