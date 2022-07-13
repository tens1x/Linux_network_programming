//利用消息队列完成了回射客户端
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
        
#define MSG_MAX 8192

struct msgbuf {
    long mtype;       /* message type, must be > 0 */
    char mtext[1];    /* message data */
};

void echo_cli(int msgid){
    
    int pid = getpid();
    struct msgbuf msg;
    memset(&msg, 0, sizeof msg);
    msg.mtype = 1;
    while(fgets(msg.mtext + 4, MSG_MAX, stdin) !=  NULL){

        *((int*)msg.mtext) = pid;//将pid保存到了结构体的前四个字节
        int n;
        if(msgsnd(msgid, &msg, 4+strlen(msg.mtext + 4), 0) < 0)
            ERR_EXIT("msgsnd");

        memset(msg.mtext, 0, MSG_MAX);
        if((n = msgrcv(msgid, &msg, MSG_MAX, pid, 0)) < 0)
            ERR_EXIT("msgsnd");

        fputs(msg.mtext + 4, stdout);
        memset(msg.mtext + 4, 0, MSG_MAX - 4);
    }
}

int main(int argc, char* argv[]){
    int msgid;
    msgid = msgget(1234, 0);
    if (msgid == -1)
        ERR_EXIT("msgget");
    echo_cli(msgid);
    return 0;
}