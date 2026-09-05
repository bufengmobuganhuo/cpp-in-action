//
// Created by yuzhang on 2026/8/10.
//
#include <complex>
#include <cstring>

#include "include/InetAddress.h"
#include "include/TcpServer.h"
#include "service/EchoServer.h"

int main(int argc, char* argv[])
{
    TcpServer server(argv[1], atoi(argv[2]));
    server.start();
}
