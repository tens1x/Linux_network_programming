#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

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

int main(){
    
    //命名规则必须要以斜杠开始
    mqd_t mqid = mq_open("/abc", O_RDONLY);//第四个参数是消息队列的属性
    if(mqid == (mqd_t)-1)
        ERR_EXIT("mq_open");

    STU stu;
    struct mq_attr attr;
    mq_getattr(mqid, &attr);
    size_t size = attr.mq_msgsize;

    unsigned prio;
    if(mq_receive(mqid, (char*)&stu, size, &prio) == (mqd_t)-1)
        ERR_EXIT("mq_receive");

    printf("name = %s age = %d prio = %u \n", stu.name, stu.age, prio);

    mq_close(mqid);
    return 0;

}
