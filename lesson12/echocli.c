#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>//TCP值的来源
#include <string.h>

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);

void echo_cli(int fd){
    //初始化一个地址，绑定它
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};
    while(fgets(sendbuf, sizeof sendbuf, stdin) != NULL){
        sendto(fd, sendbuf, strlen(sendbuf), 0 ,(struct sockaddr*)&servaddr, sizeof servaddr);
        recvfrom(fd, recvbuf, sizeof recvbuf, 0, NULL, NULL);//因为不在乎哪来的数据，所以地址填写null

        fputs(recvbuf ,stdout);
        memset(sendbuf, 0, sizeof sendbuf);
        memset(recvbuf, 0, sizeof recvbuf);
    }
    close(fd);

}

int main(){
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("sock");

    echo_cli(sock);

    return 0;
}