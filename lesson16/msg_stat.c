#include <stdio.h>
#include <unistd.h>//read write
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define ERR_EXIT(m) \
        do \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while (0);

int main(){

    int msgid;
    msgid = msgget(1234, 0);//如果想ipc创建成功需要与上IPC_CREAT，0666代表皆可访问和修改
    //msgid = msgget(IPC_PRIVATE, 0666 );
    if( msgid == -1){
        printf("%s \n", strerror(errno));
        ERR_EXIT("msgget");
    }
    printf("msgget success\n");

    struct msqid_ds buf;
    msgctl(msgid, IPC_STAT, &buf);
    printf("mode : %o \n", buf.msg_perm.mode); 
    return 0;
}