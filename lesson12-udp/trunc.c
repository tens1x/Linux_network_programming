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

void echo_srv(int fd){
    char recvbuf[1024];
    struct sockaddr_in peeraddr;
    socklen_t peerlen;
    int ret;
    while(1){
        peerlen = sizeof(peeraddr);
        memset(recvbuf, 0, sizeof recvbuf);
        ret = recvfrom(fd, recvbuf, sizeof recvbuf, 0, (struct sockaddr*)&peeraddr, &peerlen);
        if(ret == -1){
            if(errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }
        if(ret == 0){
            ERR_EXIT("client close");
            break;
        }
        else if(ret > 0){
            fputs(recvbuf, stdout);
            sendto(fd, recvbuf, ret, 0, (struct sockaddr*)&peeraddr, peerlen);
        }
    }
    close(fd);
}

int main(){
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("sock");

    //初始化一个地址，绑定它
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock, (struct sockaddr*)&servaddr, sizeof servaddr) < 0)
        ERR_EXIT("bind");
    
    sendto(sock, "hello", sizeof "hello", 0, (struct sockaddr*)&servaddr, sizeof servaddr);
    printf("sizeof : %ld \n", sizeof "hello");
    //echo_srv(sock);
    char recvbuf[1];
    for(int i = 0; i < 4; i++){
        int n = recvfrom(sock, recvbuf, 1, 0, NULL, NULL);
        if(n == -1){
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }
        else if(n > 0){
            printf("n = %d  %c \n", n, recvbuf[0]);
        }
        
    }

    return 0;
}