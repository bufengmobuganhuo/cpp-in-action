//
// Created by yuzhang on 2026/8/11.
//

#include "./include/InetAddress.h"

/*
class InetAddress
{
private:
    sockaddr_in addr_;
public:
    InetAddress(const std::string &ip, uint16_t port);
    InetAddress(const sockaddr_in addr): addr_(addr){};
    ~InetAddress();

    const char* ip() const; // 返回字符串表示的地址
    uint16_t port() const; // 返回整数表示的端口
    const sockaddr* addr() const; // 返回addr_成员的地址

};
*/

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const sockaddr_in addr) : addr_(addr)
{
}

InetAddress::~InetAddress()
{

}

const char* InetAddress::ip() const
{
    // 返回IP地址的字符串表示
    // 修正：原代码错误地返回了端口，应返回IP地址
    return inet_ntoa(addr_.sin_addr);
}

uint16_t InetAddress::port() const
{
    return ntohs(addr_.sin_port);
}

const sockaddr* InetAddress::addr() const
{
    return (sockaddr*) &addr_;
}
