//回射服务器
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>//TCP值的来源
#include <unistd.h>//read write
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

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
// void handle_sigchild(int sig){
//     waitpid(-1, NULL, WNOHANG);
// }

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
    int conn;
    socklen_t peerlen = sizeof peeraddr;

/*
    // 6. 数据交换
    pid_t pid;
    while (1) {
        int connfd;
        if ((connfd = accept(listenfd, (struct sockaddr *) &peeraddr, &peerlen)) < 0) {
            ERR_EXIT("accept");
        }

        printf("id = %s, ", inet_ntoa(peeraddr.sin_addr));
        printf("port = %d\n", ntohs(peeraddr.sin_port));

        pid = fork();

        if (pid == -1) {
            ERR_EXIT("fork");
        }
        if (pid == 0)   // 子进程
        {
            close(listenfd);
            echo_srv(connfd);
            //printf("child exit\n");
            exit(EXIT_SUCCESS);
        } else {
            //printf("parent exit\n");
            close(connfd);
        }

    }
    */

    int client[FD_SETSIZE];
    int maxi = 0;
    for(int i = 0; i<FD_SETSIZE; i++){
        client[i] = -1;
    }
    int nready, i = 0;
    int maxfd = listenfd;
    fd_set rset;
    fd_set allset;
    FD_ZERO(&rset);
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);//将监听套接口放入集合中
    while(1){
        rset = allset;
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);//select检测是否产生事件。如果产生则返回1
        if(nready == -1){
            if(errno == EINTR)
                continue; 
            ERR_EXIT("select");
        }
        if(nready == 0){
            //超时
            continue;
        }
        if(FD_ISSET(listenfd, &rset))//判定哪一个套接口发生了事件。判断监听套接口是否在集合当中。
        {
            peerlen = sizeof peeraddr;
            conn = accept(listenfd, (struct sockaddr *)&peeraddr, &peerlen);//如果在集合当中就说明发生了可读事件
            if(conn == -1)
                ERR_EXIT("accept");
            
            //为了维护多个客户端与服务端的连接，需要将其放入一个数组当中。
            //这里的listenfd根本没有进入，select根本没有监听到
            for(i = 0; i<FD_SETSIZE; i++){
                if(client[i] < 0){
                    client[i] = conn;
                    if (i > maxi){
                        maxi = i;
                        printf("maxi = %d\n", maxi);
                    }
                    break;
                }
            }
            //FD_SETSIZR设定为一个进程能够打开的最多的IO数
            if(i == FD_SETSIZE){
                fprintf(stderr, "too many clients\n");
                exit(EXIT_FAILURE);
            }
            printf("id = %s, ", inet_ntoa(peeraddr.sin_addr));
            printf("port = %d\n", ntohs(peeraddr.sin_port));

            FD_SET(conn, &allset);//往集合中添加conn
            if (conn > maxfd)
                maxfd = conn;
            if(--nready <= 0)//将待处理数减一
                continue;
        }
        for(int i = 0; i<FD_SETSIZE; i++){//select还有可能是conn事件
            printf("maxi = %d\n", maxi);
            conn = client[i];//判断是否待处理
            if(conn == -1)
                continue;
            if(FD_ISSET(conn, &rset)){//判断是否在集合中
                char recvbuf[1024] = {0};
                int ret = readline(conn, recvbuf, 1024);
                if (ret == -1)
                {
                    ERR_EXIT("readline");
                }
                if (ret == 0)
                {
                    printf("client close\n");
                    FD_CLR(conn, &allset);//从allset清除
                    client[i] = -1;
                }

                fputs(recvbuf, stdout);
                sleep(4);
                writen(conn, recvbuf, strlen(recvbuf));

                if(--nready <= 0)//所有的都处理完了
                    break;
            }
        }
    }
    // 7. 断开连接
    close(listenfd);


    return 0;
}