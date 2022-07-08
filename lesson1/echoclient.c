//回射客户端 cant work
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
    
int main(){
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)//定义套接字
        ERR_EXIT("Sockt fail");

    //地址初试化
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;//ipv4,这里不使用pf_inet。一般af用于定义class
    servaddr.sin_port = htons(6666);//从主机字节序到网络字节序
    /*servaddr.sin_addr.s_addr = htonl(INADDR_ANY);*///随机分配一个地址
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //指定一个网络地址
    /*inet_aton("127.0.0.1", &servaddr.sin_addr);*/

    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("connect error");
    else    
        printf("success \n");

    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};

    while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL){
        write(sockfd, sendbuf, sizeof(sendbuf));
        read(sockfd, recvbuf, sizeof(recvbuf));

        fputs(recvbuf, stdout);
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }
    close(sockfd);

    return 0;
}