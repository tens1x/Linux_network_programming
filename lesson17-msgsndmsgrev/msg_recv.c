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
    msgid = msgget(1234, 0);
    if(msgid == -1)
        ERR_EXIT("msgget");
    struct msgbuf *ptr;
    ptr = (struct msgbuf*)malloc(sizeof(long) + MSG_MAX);
    ptr->mtype = type;
    int nrecv = 0;
    if((nrecv = msgrcv(msgid, ptr, MSG_MAX, type, flag)) < 0)
        ERR_EXIT("msgrcv");

    printf("read %d bytes type = %ld \n", nrecv, ptr->mtype);
        
    return 0;
}