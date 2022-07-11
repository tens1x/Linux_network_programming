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

void echo_srv(int conn){
    
    char recvbuf[1024];
    int ret;
    while(1){
        memset(recvbuf, 0, sizeof recvbuf);
        ret = read(conn, recvbuf, sizeof recvbuf);
        if( ret == -1)
        {
            if( ret == EINTR)
                continue;
            ERR_EXIT("read");
        }
        else if(ret == 0){
            printf("client close");
            close(conn);
            break;
        }
        fputs(recvbuf, stdout);
        write(conn, recvbuf, strlen(recvbuf));
        close(conn);
    }
}


int main(){
    int sock;
    if((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
        ERR_EXIT("socket");

    unlink("test_socket");
    
    struct sockaddr_un servaddr;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, "test_socket");
    
    if(bind(sock, (struct sockaddr*)&servaddr, sizeof servaddr) < 0)
        ERR_EXIT("bind");

    if(listen(sock, SOMAXCONN) < 0) 
        ERR_EXIT("listen");

    int conn;
    pid_t pid;

    while(1){
        conn = accept(sock, NULL, NULL);
        if(conn == -1){
            if(errno == EINTR)
                continue;
            ERR_EXIT("accept");
        }

        pid = fork();
        if(pid == -1)
            ERR_EXIT("fork");
        if(pid == 0)//子进程
        {
            close(sock);
            echo_srv(conn);
            exit(EXIT_SUCCESS);
        }
        if(pid > 0)//父进程
        {
            close(conn);
        }


    }



    return 0;
}