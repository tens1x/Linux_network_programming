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

int main(){

    int sockfds[2];
    if (socketpair(PF_UNIX, SOCK_STREAM, 0 , sockfds) < 0){
        ERR_EXIT("socket");
    }
    pid_t pid;
    pid = fork();
    if(pid == -1)
        ERR_EXIT("fork");
    if(pid > 0){
        int val = 0;
        close(sockfds[1]);
        while(1){
            ++val;
            printf("sending data val : %d\n", val);
            write(sockfds[0], &val, sizeof val);
            read(sockfds[0], &val, sizeof val);
            printf("data received: %d \n", val);
            sleep(1);
        }
    }
    else if(pid == 0){
        int val;
        close(sockfds[0]);
        while(1){
            read(sockfds[1], &val, sizeof val);
            val++;
            write(sockfds[1], &val, sizeof val);
        }
    }

    return 0;
}

