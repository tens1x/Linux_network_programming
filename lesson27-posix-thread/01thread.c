#include<unistd.h>
#include<sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define ERR_EXIT(m) \
        do \
        { \
             perror(m); \
             exit(EXIT_FAILURE);    \
        } while (0)

void* thread_routine(void *arg)
{
    for(int i = 0; i < 20; i++){
        printf("B\n");
        fflush(stdout);
        usleep(20);
    }
    return 0;
}

int main(){

    pthread_t tid;
    int ret = pthread_create(&tid, NULL, thread_routine, NULL);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_create:%s \n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < 20; i++){
        printf("A\n");
        fflush(stdout);
        usleep(20);
    }

    if((ret = pthread_join(tid, NULL)) ! =0)//waitpid
    {
        fprintf(stderr, "pthread_join:%s \n", strerror(ret));
        exit(EXIT_FAILURE);
    }
    sleep(2);
        
    return 0;
}
