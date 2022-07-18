//实现信号量互斥
//哲学家进餐问题
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

#define ERR_EXIT(m) \
        do \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while (0);

#define DELAY ((rand() % 5 + 1))

#define Pnum 5

int sem_p(int semid)
{
    //对第几个信号量进行操作，-1，默认选项
    struct sembuf sb = {0, -1, 0};
    int ret = semop(semid, &sb, 1);
    if(ret == -1)
        ERR_EXIT("semop");
    return ret;
}

int sem_v(int semid)
{
    //对第几个信号量进行操作，-1，默认选项
    struct sembuf sb = {0, 1, 0};
    int ret = semop(semid, &sb, 1);
    if(ret == -1)
        ERR_EXIT("semop");

    return ret;
}

int semid;

void wait_for_2forks(int no){
    int left = no;
    int right = ( no + 1) % Pnum;

    struct sembuf buf[2] = {{left, -1, 0}, {right, -1 ,0}};
    int ret = semop(semid ,buf, 2);
    if(ret == -1)
        ERR_EXIT("wait_for_2forks");

}

void free_2forks(int no){ b
    int left = no;
    int right = ( no + 1) % Pnum;

    struct sembuf buf[2] = {{left, 1, 0}, {right, 1 ,0}};
    int ret = semop(semid ,buf, 2);
    if(ret == -1)
        ERR_EXIT("free_2forks");
}

void Philosophere(int no){
    srand(getpid());
    for(;;){
        printf("%d is thinking... \n", no);
        sleep(DELAY);
        printf("%d is hungry \n", no);
        wait_for_2forks(no);
        printf("%d is eating\n", no);
        sleep(DELAY);
        free_2forks(no);
    }
}
int main(int argc, char* argv[])
{
    semid = semget(IPC_PRIVATE, 5, IPC_CREAT | 0666);
    if (semid == -1)
        ERR_EXIT("semget");
    union semun su;
    su.val = 1;
    for(int i = 0; i < 5; i++){
        semctl(semid, i , SETVAL, su);
    }

    int no = 0;
    pid_t pid;
    for(int i = 1; i < 5; i++)//create four child progress
    {
        pid = fork();
        if (pid == -1)
            ERR_EXIT("fork");
        if(pid == 0){
            no = i;
            break;
        }
    }

    Philosophere(no);
    return 0;
}