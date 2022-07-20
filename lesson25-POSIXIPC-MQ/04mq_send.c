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

int main(int argc, char* argv[]){
    
    if (argc != 2){
        fprintf(stderr, "Usage : %s <prio>\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    //命名规则必须要以斜杠开始
    mqd_t mqid = mq_open("/abc", O_WRONLY);//第四个参数是消息队列的属性
    if(mqid == (mqd_t)-1)
        ERR_EXIT("mq_open");

    STU stu;
    strcpy(stu.name, "Jack");
    stu.age = 20;

    unsigned prio = atoi(argv[1]);
    mq_send(mqid, (const char*)&stu, sizeof(stu), prio);

    mq_close(mqid);
    return 0;

}
