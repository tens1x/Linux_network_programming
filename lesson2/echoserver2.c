//回射服务器 增加fork
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>//TCP值的来源
#include <unistd.h>//read write
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
    do \
    { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

void do_service(int conn){
     char recvbuf[1024];
    //不断接收
    while(1){
        memset(recvbuf, 0, sizeof(recvbuf));
        int ret = read(conn, recvbuf, sizeof(recvbuf));
        if(ret == 0){
            printf("conn closed");
            break;
        }
        fputs(recvbuf, stdout);
        write(conn, recvbuf, ret);
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
    int conn;

    pid_t pid;
    while(1){
        if ((conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen)) < 0 ) //转为被动连接
            ERR_EXIT("accept error");
        printf("ip=%s port=%d \n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
        pid = fork();
        if(pid == -1)
            ERR_EXIT("fork error");
        if(pid == 0){//子进程
            close(listenfd);
            do_service(conn);
            exit(EXIT_SUCCESS);//销毁pid这个进程
        }
        else
            close(conn);
    }

    return 0;
}