//
// Created by yuzhang on 2026/8/11.
//

#ifndef REACTOR_SERVER_INETADDRESS_H
#define REACTOR_SERVER_INETADDRESS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
class InetAddress
{
private:
    sockaddr_in addr_{};
public:
    InetAddress(const std::string &ip, uint16_t port);
    InetAddress(const sockaddr_in addr);
    ~InetAddress();

    const char* ip() const; // 返回字符串表示的地址
    uint16_t port() const; // 返回整数表示的端口
    const sockaddr* addr() const; // 返回addr_成员的地址

};


#endif //REACTOR_SERVER_INETADDRESS_H
