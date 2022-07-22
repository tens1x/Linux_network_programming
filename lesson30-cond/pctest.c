#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>


#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0)

#define CONSUMERS_COUNT 1
#define PRODUCERS_COUNT 5

pthread_mutex_t g_mutex;
pthread_cond_t cond;

pthread_t g_thread[CONSUMERS_COUNT + PRODUCERS_COUNT];

int nready = 0;

void* consumer(void* arg)
{
    int num = *((int*)arg);
    
    while (1)
    {
        pthread_mutex_lock(&g_mutex);
        while(nready == 0){
            printf("%d begin wait a condtion\n", num);
            //wait可能会自动重启和虚假唤醒，可能会退出wait状态。所以需要while循环来不断判断，保护信号量正确。
            /*再wait函数内部
            1、对mutex解锁
                a.可以使别的进程访问临界区变量
                b.可以使别的进程进行等待
            2、等待条件，直到有线程向他发起通知
            3、对mutex上锁
            */
            pthread_cond_wait(&cond, &g_mutex);
        }
        printf("%d end wait a condition\n", num);
        --nready;
        pthread_mutex_unlock(&g_mutex);
        sleep(1);
    }
    return NULL;
}

void* producer(void* arg)
{
    int num = *((int*)arg);
    while (1)
    {
        pthread_mutex_lock(&g_mutex);
        ++nready;
        pthread_cond_signal(&cond);
        printf("%d signal \n", num);
        pthread_mutex_unlock(&g_mutex);
        sleep(1);
    }
    
    return NULL;
}

int main(){

    pthread_mutex_init(&g_mutex, NULL);
    pthread_cond_init(&cond, NULL);

    for(int i = 0; i < CONSUMERS_COUNT; i++)
    {
        pthread_create(&g_thread[i], NULL, consumer, &i);
    }

    for(int i = 0; i < PRODUCERS_COUNT; i++)
    {
        pthread_create(&g_thread[CONSUMERS_COUNT + i], NULL, producer, &i);
    }

    for(int i = 0; i < CONSUMERS_COUNT + PRODUCERS_COUNT; i++)
    {
        pthread_join(g_thread[i], NULL);
    }
    pthread_mutex_destroy(&g_mutex);
    pthread_cond_destroy(&cond);
    return 0;
}