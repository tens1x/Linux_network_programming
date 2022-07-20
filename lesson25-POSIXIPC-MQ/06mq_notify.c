//每一次注册，消息队列从空到有消息才会通知
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
        do \
        { \
             perror(m); \
             exit(EXIT_FAILURE);    \
        } while (0)

typedef struct stu {
    char name[32];
    int age;
}STU;

size_t size;
mqd_t mqid;
struct sigevent sigev;
void handle_sigusr1(int sig){
    mq_notify(mqid, &sigev);//重新调用notify可以重新注册。并且要放在receive之前。
    STU stu;
    unsigned prio;
    if(mq_receive(mqid, (char *)&stu, size, &prio) == (mqd_t)-1)
        ERR_EXIT("mq_receive");
    printf("name = %s age = %d prio = %u \n", stu.name, stu.age, prio);
}

int main(){
    
    //命名规则必须要以斜杠开始
    mqid = mq_open("/abc", O_RDONLY);//第四个参数是消息队列的属性
    if(mqid == (mqd_t)-1)
        ERR_EXIT("mq_open");

    signal(SIGUSR1, handle_sigusr1);

    struct mq_attr attr;
    mq_getattr(mqid, &attr);
    size = attr.mq_msgsize;

    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGUSR1;//注册了一个通知
    mq_notify(mqid, &sigev);

    while(1){
        pause();
    }

    mq_close(mqid);
    return 0;

}
