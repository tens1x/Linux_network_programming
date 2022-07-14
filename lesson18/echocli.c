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
    char mtext[MSG_MAX];    /* message data */
};

void echo_cli(int msgid){
    
    int pid = getpid();//得到进程的pid
    struct msgbuf msg;//需要发送的msg结构体
    memset(&msg, 0, sizeof msg);//清空
    msg.mtype = 1;//定义约定好的类型。只接受和发送type=1的消息
    *((int*)msg.mtext) = pid;//将pid保存到了结构体的前四个字节
    int n;
    while(fgets(msg.mtext + 4, MSG_MAX, stdin) !=  NULL){//前四个字节为pid，从后四个字节开始
        if(msgsnd(msgid, &msg, 4+strlen(msg.mtext + 4), 0) < 0)//向消息队列msgid发送数据，其中mtype=1
            ERR_EXIT("msgsnd");

        memset(msg.mtext, 0, sizeof(msg.mtext + 4));//清空
        if((n = msgrcv(msgid, &msg, MSG_MAX, pid, 0)) < 0)//从消息队列接收msgtyp=pid的数据
            ERR_EXIT("msgsnd");
        fputs(msg.mtext + 4, stdout);//从后四位开始发送
        memset(msg.mtext + 4, 0, MSG_MAX - 4);//清空
    }
}


int main(int argc, char* argv[]){
    int msgid,msgcli;
    msgid = msgget(1234, 0);
    if (msgid == -1)
        ERR_EXIT("msgget msgid");
    if (msgcli == -1)
        ERR_EXIT("msgget msgcli");
    echo_cli(msgid);
    return 0;
}