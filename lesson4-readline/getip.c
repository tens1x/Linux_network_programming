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
#include <netdb.h>

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);

int getlocalip(char *ip){
    char host[100] = {0};
    if (gethostname(host, sizeof host) < 0)
        ERR_EXIT("gethostname");
    struct hostent *hp;
    if ((hp = gethostbyname(host)) == NULL)
        return -1;
    strcpy(ip, inet_ntoa(*(struct in_addr*)hp->h_addr_list[0]));
    return 0;
}


int main(){
    char host[100] = {0};
    if (gethostname(host, sizeof host) < 0)
        ERR_EXIT("gethostname");
    struct hostent *hp;
    if ((hp = gethostbyname(host)) == NULL)
     ERR_EXIT("gethostbyname");

    int i=0;
    while(hp->h_addr_list[i] != NULL){
        printf("%s\n", inet_ntoa(*(struct in_addr*)hp->h_addr_list[i]));
        i++;
    }     

    char ip[16] = {0};
    getlocalip(ip);
    printf("%s \n", ip);
    return 0;

}