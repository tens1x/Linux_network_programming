#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define ERR_EXIT(m) \
        do \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while (0);

typedef struct stu{
    char name[32];
    int age;
}STU;

int main(int argc, char* argv[])
{
    int shmid = shmget(1234, sizeof(STU), IPC_CREAT | 0666);
    if( shmid == -1)
        ERR_EXIT("shmget");

    STU *p;
    p = shmat(shmid, NULL, 0);
    if (p == (void*)(-1))
        ERR_EXIT("shmat");
    strcpy(p->name, "Jack");//这相当于对共享内存进行了操作
    p->age = 20;

    while(1){
        if(memcmp(p, "quit", 4) == 0)
        {
            shmctl(shmid, IPC_RMID, NULL);//删除共享内存段
            break;
        }
    }

    shmdt(p);//解除映射
    return 0;   
}