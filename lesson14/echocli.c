#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>//read write
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/un.h>

#define ERR_EXIT(m) \
        do \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while (0);

void echo_cli(int sockfd){

    char recvbuf[1024] = {0};
    char sendbuf[1024] = {0};
    while(fgets(sendbuf, sizeof recvbuf, stdin) != NULL){
        write(sockfd, sendbuf, strlen(sendbuf));
        read(sockfd, recvbuf, sizeof(recvbuf));
        fputs(recvbuf, stdout);
        memset(recvbuf, 0, sizeof recvbuf);
        memset(sendbuf, 0, sizeof sendbuf);
    }
    close(sockfd);
}


int main()
{
    int sock;
    if((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
        ERR_EXIT("socket");
    
    struct sockaddr_un servaddr;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, "test socket");

    if(connect(sock, (struct sockaddr*)&servaddr, sizeof servaddr) < 0)
        ERR_EXIT("connect");

    echo_cli(sock);
    return 0;
}