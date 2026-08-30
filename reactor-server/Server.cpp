//
// Created by yuzhang on 2026/8/10.
//
#include <complex>
#include <cstring>

#include "include/InetAddress.h"
#include "include/TcpServer.h"

int main(int argc, char* argv[])
{
    TcpServer tcpServer(argv[1], atoi(argv[2]));
    tcpServer.start();
}
