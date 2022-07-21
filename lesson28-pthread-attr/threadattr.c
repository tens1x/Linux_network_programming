#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);

int main(){
    pthread_attr_t attr;
    pthread_attr_init(&attr);//初试化属性值。初始化后包括将包括很多默认值。

    int state;
    pthread_attr_getdetachstate(&attr, &state);//获取线程分离属性的默认值，保存早state中
    if(state == PTHREAD_CREATE_JOINABLE)
        printf("detachstate:PTHREAD_CREATE_JOINABLE\n");
    else if(state == PTHREAD_CREATE_DETACHED)
        printf("detachstate:PTHREAD_CREATE_DETACHED\n");

    size_t size;
    pthread_attr_getstacksize(&attr, &size);
    printf("stacksize : %ld \n", size);

    pthread_attr_getguardsize(&attr, &size);
    printf("guardsize : %ld \n", size);

    int scope;
    pthread_attr_getscope(&attr, &scope);//获取线程竞争范围。系统进程还是用户进程
    if(scope == PTHREAD_SCOPE_PROCESS)
        printf("scope:PTHREAD_SCOPE_PROCESS\n");
    else if(scope == PTHREAD_SCOPE_SYSTEM)
        printf("scope:PTHREAD_SCOPE_SYSTEM\n");

    int policy;
    pthread_attr_getschedpolicy(&attr, &policy);//调度策略
    if(policy == SCHED_FIFO)
        printf("policy:SCHED_FIFO\n");
    else if(policy == SCHED_RR)//优先级相同，抢占式调度
        printf("policy:SCHED_RR\n");
    else if(policy == SCHED_OTHER)
        printf("policy:SCHED_OTHER\n");

    int inheritsched;
    pthread_attr_getinheritsched(&attr, &inheritsched);//调度策略是否继承
    if(inheritsched == PTHREAD_INHERIT_SCHED)
        printf("inheritsched:PTHREAD_INHERIT_SCHED\n");
    else if(inheritsched == PTHREAD_EXPLICIT_SCHED)
        printf("inheritsched:PTHREAD_EXPLICIT_SCHED\n");
    
    struct sched_param param;
    pthread_attr_getschedparam(&attr, &param);//调度参数
    printf("sched_priority: %d \n", param.sched_priority);//线程优先级

    pthread_attr_destroy(&attr);

    int level = pthread_getconcurrency();//并发级别
    printf("level: %d\n", level);

    return 0;
    

}