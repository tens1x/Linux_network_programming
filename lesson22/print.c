//实现信号量互斥打印例子
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

int sem_create(key_t key)
{
    int semid;
    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);//ipc_excl不允许创建两次
    if(semid == -1)
        ERR_EXIT("sem_create");
    return semid;
}

int sem_open(key_t key)
{
    int semid = semget(key, 0, 0);
    if(semid == -1)
        ERR_EXIT("sem_open");

    return semid;
    
}

int sem_setval(int semid, int val){
    union semun su;
    su.val = val;
    //id,哪个信号量操作，命令
    int ret = semctl(semid, 0, SETVAL, su);
    if( ret == -1)
        ERR_EXIT("sem_setval");  
    return 0;
}

int sem_getval(int semid){
    //id,哪个信号量操作，命令
    int ret = semctl(semid, 0, GETVAL);
    if( ret == -1)
        ERR_EXIT("sem_getval");

    return ret;  
}

int sem_del(int semid)
{
    int ret = semctl(semid, 0, IPC_RMID, 0 );
    if( ret == -1)
        ERR_EXIT("sem_del");
    return 0;
}

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

void print(char op_char){
    int pause_time;
    srand(getpid());
    int i;
    for(i = 0; i < 10; i++){
        sem_p(semid);
        printf("first: %c \n", op_char);
        //fflush(stdout);//因为加了\n，stdout被清空
        pause_time = rand() % 3;
        printf("second: %c \n", op_char);
        sleep(pause_time);
        sem_v(semid);
    }
}

int main(int argc, char* argv[])
{
    semid = sem_create(IPC_PRIVATE);
    sem_setval(semid, 1);
    pid_t pid;
    pid = fork();
    if(pid == -1){
        ERR_EXIT("fork");
    }
    if (pid > 0)
    {   
        //sem_setval(semid, 1);
        print('o');
    }
    else{
        print('x');
    }
    
    return 0;
}