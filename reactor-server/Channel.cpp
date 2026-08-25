//
// Created by yuzhang on 2026/8/18.
//
#include "include/Channel.h"

Channel::Channel(Epoll* epoll, int fd, bool isListen) : ep_(epoll), fd_(fd), isListen(isListen)
{
}

Channel::~Channel()
{
}

int Channel::fd() const
{
    return fd_;
}

void Channel::useET()
{
    events_ = events_ | EPOLLET;
}

void Channel::enableReading()
{
    events_ = events_ | EPOLLIN;
    ep_->updateChannel(this);
}

void Channel::setInEpoll()
{
    inEpoll_ = true;
}

void Channel::setReadyEvent(uint32_t event)
{
    readyEvents_ = event;
}

bool Channel::inEpoll() const
{
    return inEpoll_;
}

uint32_t Channel::events() const
{
    return events_;
}

uint32_t Channel::readyEvents() const
{
    return readyEvents_;
}

void Channel::handleEvent(Socket& serv_socket)
{
    if (events_ & EPOLLRDHUP)
    {
        // 对方已关闭
        printf("[Server] event_fd=%d is closed", fd_);
        close(fd_);
    }
    else if (events_ & (EPOLLIN | EPOLLPRI))
    {
        // 处理新连接
        if (isListen)
        {
            InetAddress client_addr;
            auto* client_socket = new Socket(serv_socket.accept(client_addr));
            printf("accept client(fd=%d,ip=%s,port=%d) ok.\n", client_socket->fd(), client_addr.ip(),
                   client_addr.port());
            Channel* client_channel = new Channel(ep_, client_socket->fd(), false);
            client_channel->useET();
            client_channel->enableReading();
        }
        else
        {
            // 客户端有数据可读，要一直读完为止
            // 处理客户端数据
            char buffer[kBufferSize] = {0};
            while (true)
            {
                // 清理buffer
                bzero(&buffer, sizeof(buffer));
                int bytes_read = read(fd_, buffer, kBufferSize - 1);
                if (bytes_read > 0)
                {
                    buffer[bytes_read] = '\0';
                    std::cout << "[EpollServer] Received: " << buffer << std::endl;
                    send(fd_, buffer, bytes_read, 0);
                }
                else if (bytes_read == 0)
                {
                    // 客户端连接已断开
                    std::cout << "[Server] Client disconnected, fd: " << fd_ << std::endl;
                    close(fd_);
                    break;
                }
                else if (bytes_read == -1 && errno == EINTR)
                {
                    // 读取数据的时候被信号中断，继续读取
                    continue;
                }
                else if (bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                {
                    // 全部数据已读取完毕
                    break;
                }
            }
        }
    }
    else if (events_ & EPOLLOUT)
    {
        // 写事件准备好
    }
    else
    {
        // 其他事件都认为是错误
        std::cout << "[Server] unknow event: " << events_ << "for client: " << fd_ << std::endl;
        close(fd_);
    }
}
