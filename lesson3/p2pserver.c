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

#define ERR_EXIT(m) \
    do \
    { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

struct packet
{
    /* data */
    int len;
    char buf[1024];
};


void handler(int sig){
    printf("recv a sig=%d \n", sig);
    exit(EXIT_SUCCESS);
}


ssize_t readn(int fd, void *buf, size_t count){
    size_t nleft = count;
    ssize_t nread;
    char* bufp = (char*)buf;
    while(nleft > 0){
        if((nread = read(fd, bufp, nleft)) < 0){
            if(errno == EINTR)
                continue;
            return -1;
        }
        else if(nread == 0)//对方关闭
            return count - nleft; //已经读取的字节数
        else
            bufp += nread;//偏移
            nleft -= nread;//剩余要读取的字节数
    }

    return count;
}

ssize_t writen(int fd, const void *buf, size_t count){
    size_t nleft = count;
    ssize_t nwritten;
    char* bufp = (char*)buf;
    while(nleft > 0){
        if((nwritten = write(fd, bufp, nleft)) < 0){//信号中断
            if(errno == EINTR)
                continue;
            return -1;
        }
        else if(nwritten == 0)//对方关闭
            continue;
        else
            bufp += nwritten;//偏移
            nleft -= nwritten;//剩余要读取的字节数
    }

    return count;
}

void do_service(int conn){
     struct packet recvbuf;
    //不断接收
    while(1){
        memset(&recvbuf, 0, sizeof(recvbuf));
        int ret = readn(conn, &recvbuf.len, 4);
        if (ret == -1)
            ERR_EXIT("read failure");
        else if(ret < 4){
            printf("connect closed");
            break;
        }
        int n = ntohl(recvbuf.len);
        ret = readn(conn, recvbuf.buf, n);
        if(ret == -1) 
            ERR_EXIT("read error");
        else if(ret < n){
            printf("client closed");
            break;
        }
        fputs(recvbuf.buf, stdout);
        writen(conn, &recvbuf, 4+n);
    }
}  
    


int main(int argc, char** argv){
    int listenfd;
    if((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)//定义套接字
        ERR_EXIT("Sockt fail");

    //地址初试化
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;//ipv4,这里不使用pf_inet。一般af用于定义class
    servaddr.sin_port = htons(6666);//从主机字节序到网络字节序
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//随机分配一个地址
    //servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //指定一个网络地址
    /*inet_aton("127.0.0.1", &servaddr.sin_addr);*/

    int on = 1;
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) <0 )
        ERR_EXIT("setsockopt");

    //将套接字与地址的绑定
    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind error");

    //listen函数，转化为监听状态
    //SOMAXCONN为backlog参数，tcp中两队列长度之和小于等于backlog
    if(listen(listenfd, SOMAXCONN) < 0) 
        ERR_EXIT("listen error");

    struct sockaddr_in peeraddr;//定义接收地址
    socklen_t peerlen = sizeof(peeraddr);

    struct packet sendbuf;
    struct packet recvbuf;
    memset(&sendbuf, 0, sizeof(sendbuf));
    memset(&recvbuf, 0, sizeof(recvbuf));

    int conn;
    if ((conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen)) < 0 ) //转为被动连接
        ERR_EXIT("accept error");
    printf("ip=%s port=%d \n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
    
    pid_t pid;
    pid = fork();
    if( pid == -1)
        ERR_EXIT("fork error");
    if( pid == 0){
        //write
        signal(SIGUSR1, handler);
        //char sendbuf[1024] = {0};
        int n;
        while(fgets(sendbuf.buf, sizeof(sendbuf.buf), stdin) != NULL){
            n = strlen(sendbuf.buf);
            sendbuf.len = htonl(n);
            writen(conn, &sendbuf, 4+n);//4+n
            memset(&sendbuf, 0, sizeof sendbuf);
        }
        exit(EXIT_SUCCESS);
    }
    else{
        //the parent progress,read
        while(1){
            memset(&recvbuf, 0, sizeof(recvbuf));
            int ret = readn(conn, &recvbuf.len, 4);
            if(ret == -1)
                ERR_EXIT("read error");
            else if(ret < 4){
                printf("peer closed\n");\
                break;
            }

            int n = ntohl(recvbuf.len);
            ret = readn(conn, recvbuf.buf, n);
            if(ret == -1)
                ERR_EXIT("read error");
            else if(ret < n){
                printf("peer closed\n");\
                break;
            }
            fputs(recvbuf.buf, stdout);
        }
        close(conn);
        kill(pid, SIGUSR1);
        exit(EXIT_SUCCESS);
    }
    
    return 0;
}

