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
#include <sys/resource.h>

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);

int main(){
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0){
        ERR_EXIT("getlimit");
    }
    printf("limit is %ld \n", rl.rlim_max);

    rl.rlim_cur = 2048;
    rl.rlim_max = 2048;
    if (setrlimit(RLIMIT_NOFILE, &rl) < 0)
        ERR_EXIT("set limit");
    
    printf("limit is %ld \n", rl.rlim_max);

    return 0;
}