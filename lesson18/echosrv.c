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
    char mtext[MSG_MAX];    /* message data */
};

void echo_srv(int id)
{
    struct msgbuf msg;
    memset(&msg, 0, sizeof msg);
    int n;
    while(1){
        if((n = msgrcv(id, &msg, MSG_MAX, 1, 0)) < 0)
            ERR_EXIT("msgrecv");
            int pid;
            pid = *((int*)msg.mtext);

            fputs(msg.mtext + 4, stdout);
            msg.mtype = pid;
            msgsnd(id, &msg, n, 0);
    }

}
int main(int argc, char* argv[]){

    int type = 0;
    int flag = 0;
    while(1){
        int opt = getopt(argc, argv, "nt:");
        if(opt == '?')
            exit(EXIT_FAILURE);
        if(opt == -1)
            break;
        switch (opt)
        {
        case 'n':
            flag |= IPC_NOWAIT;
            break;
        case 't':
            type = atoi(optarg);
            break;
        default:
            break;
        }    
    }
    int msgid;
    msgid = msgget(1234, 0666 | IPC_CREAT);
    if(msgid == -1)
        ERR_EXIT("msgget");
    echo_srv(msgid);
    return 0;
}