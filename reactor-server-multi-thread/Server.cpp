//
// Created by yuzhang on 2026/8/10.
//
#include <complex>
#include <cstring>

#include "include/InetAddress.h"
#include "include/TcpServer.h"
#include "service/EchoServer.h"
#include <signal.h>

service::EchoServer* server;

void stop(int signal)
{
    printf("signal=%d", signal);
    delete server;
    exit(0);
}

int main(int argc, char* argv[])
{
    signal(SIGTERM, stop);
    signal(SIGINT, stop);
    server = new service::EchoServer(argv[1], atoi(argv[2]));
    server->start();
}
