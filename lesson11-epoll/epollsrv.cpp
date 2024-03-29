
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <poll.h>
#include <vector>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>

using namespace std;

typedef vector<struct epoll_event> EventList;

struct packet
{
    int len;
    char buf[1024];
};

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);

ssize_t readn(int fd, void *buf, size_t count)
{
    size_t nleft = count;   // 剩余字节数
    ssize_t nread;
    char *bufp = (char*) buf;

    while (nleft > 0)
    {
        nread = read(fd, bufp, nleft);
        if (nread < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return  -1;
        } else if (nread == 0)
        {
            return count - nleft;
        }

        bufp += nread;
        nleft -= nread;
    }
    return count;
}

ssize_t writen(int fd, const void *buf, size_t count)
{
    size_t nleft = count;
    ssize_t nwritten;
    char* bufp = (char*)buf;

    while (nleft > 0)
    {
        if ((nwritten = write(fd, bufp, nleft)) < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return -1;
        }
        else if (nwritten == 0)
        {
            continue;
        }
        bufp += nwritten;
        nleft -= nwritten;
    }
    return count;
}

ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
    while (1)
    {
        int ret = recv(sockfd, buf, len, MSG_PEEK); // 查看传入消息
        if (ret == -1 && errno == EINTR)
        {
            continue;
        }
        return ret;
    }
}

ssize_t readline(int sockfd, void *buf, size_t maxline)
{
    int ret;
    int nread;
    char *bufp = (char*)buf;    // 当前指针位置
    int nleft = maxline;
    while (1)
    {
        ret = recv_peek(sockfd, buf, nleft);
        if (ret < 0)
        {
            return ret;
        }
        else if (ret == 0)
        {
            return ret;
        }
        nread = ret;
        int i;
        for (i = 0; i < nread; i++)
        {
            if (bufp[i] == '\n')
            {
                ret = readn(sockfd, bufp, i+1);
                if (ret != i+1)
                {
                    exit(EXIT_FAILURE);
                }
                return ret;
            }
        }
        if (nread > nleft)
        {
            exit(EXIT_FAILURE);
        }
        nleft -= nread;
        ret = readn(sockfd, bufp, nread);
        if (ret != nread)
        {
            exit(EXIT_FAILURE);
        }
        bufp += nread;
    }
    return -1;
}

void echo_srv(int connfd)
{
    char recvbuf[1024];
    // struct packet recvbuf;
    int n;
    while (1)
    {
        memset(recvbuf, 0, sizeof recvbuf);
        int ret = readline(connfd, recvbuf, 1024);
        if (ret == -1)
        {
            ERR_EXIT("readline");
        }
        if (ret == 0)
        {
            printf("client close\n");
            break;
        }

        fputs(recvbuf, stdout);
        writen(connfd, recvbuf, strlen(recvbuf));
    }

}

void activate_nonblock(int fd)
{
    int ret;
    int flags = fcntl(fd, F_GETFL);
    if(flags == -1)
        ERR_EXIT("fcntl");
    flags |= O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
    if(ret == -1)
        ERR_EXIT("fcntl");
}

int main(int argc, char** argv) {
    // 1. 创建套接字
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        ERR_EXIT("socket");
    }

    // 2. 分配套接字地址
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // inet_aton("127.0.0.1", &servaddr.sin_addr);

    int on = 1;
    // 确保time_wait状态下同一端口仍可使用
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) < 0) {
        ERR_EXIT("setsockopt");
    }

    // 3. 绑定套接字地址
    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof servaddr) < 0) {
        ERR_EXIT("bind");
    }
    // 4. 等待连接请求状态
    if (listen(listenfd, SOMAXCONN) < 0) {
        ERR_EXIT("listen");
    }
    // 5. 允许连接
    struct sockaddr_in peeraddr;
    socklen_t peerlen;


    // 6. 数据交换
    int nready;
    int connfd;
    int i;
    vector<int> clients;//客户端已连接的套接字
    int epollfd;
    epollfd = epoll_create1(EPOLL_CLOEXEC); // 创建一个多路复用的实例
    
    //将感兴趣的文件描述符加入epoll进行管理
    struct epoll_event event;
    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);//创建一个epoll的实例

    EventList events(16);

    while (1)
    {
        // 等侍注册在epfd上的socket fd的事件的发生，如果发生则将发生的sokct fd和事件类型放入到events数组中
        nready = epoll_wait(epollfd, &*events.begin(), static_cast<int>(events.size()), -1);    // 用于轮询I/O事件的发生
        if (nready == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            ERR_EXIT("epoll_wait");
        }

        if (nready == 0)
        {
            continue;
        }

        if ((size_t)nready == events.size())//空间不够用，resize它  
        {
            events.resize(events.size()*2);
        }

        for (i = 0; i < nready; ++i)//遍历事件，事件存储在events中
        {
            if (events[i].data.fd == listenfd)
            {
                peerlen = sizeof(peeraddr);
                connfd = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen);
                if (connfd == -1)
                {
                    ERR_EXIT("accept");
                }
                printf("id = %s, ", inet_ntoa(peeraddr.sin_addr));
                printf("port = %d\n", ntohs(peeraddr.sin_port));
                clients.push_back(connfd);
                activate_nonblock(connfd);//将套接字设置成非阻塞模式

                event.data.fd = connfd;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event);

            } else if (events[i].events & EPOLLIN)
            {
                connfd = events[i].data.fd;
                if (connfd < 0)
                {
                    continue;
                }
                char recvbuf[1024] = {0};
                int ret = readline(connfd, recvbuf, sizeof(recvbuf));
                if (ret == -1)
                {
                    ERR_EXIT("readline");
                }
                if (ret == 0)
                {
                    printf("client close\n");
                    close(connfd);

                    event = events[i];
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, &event);
                    clients.erase(
                            remove_if(clients.begin(), clients.end(), [connfd](int n){return n == connfd;}),
                            clients.end());
                }
                fputs(recvbuf, stdout);
                writen(connfd, recvbuf, strlen(recvbuf));
                memset(recvbuf, 0, sizeof(recvbuf));
            }
        }

    }
    // 7. 断开连接
    close(listenfd);
    return 0;
}