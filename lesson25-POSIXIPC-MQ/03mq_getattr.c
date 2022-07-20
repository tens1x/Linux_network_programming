#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define ERR_EXIT(m) \
        do \
        { \
             perror(m); \
             exit(EXIT_FAILURE);    \
        } while (0)

int main(){
    //命名规则必须要以斜杠开始
    mqd_t mqid = mq_open("/abc", O_CREAT | O_RDWR, 0666, NULL);//第四个参数是消息队列的属性
    if(mqid == (mqd_t)-1)
        ERR_EXIT("mq_open");

    mq_close(mqid);
    return 0;

}
