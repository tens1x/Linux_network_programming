#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>
#include <string.h>

#define ERR_EXIT(m) \
        do \
        { \
             perror(m); \
             exit(EXIT_FAILURE);    \
        } while (0);

typedef struct stu{
    char name[20];
    int age;
}STU;

int main(){

    int shmid = shm_open("/xyz", O_RDWR, 0);
    if(shmid == -1)
        ERR_EXIT("shm_open");

    printf("shm open success \n");

    struct stat st;
    if(fstat(shmid, &st) == -1)//获取消息队列状态
        ERR_EXIT("fstat \n");

    STU *p;
    p  = mmap(NULL, st.st_size, PROT_WRITE, MAP_SHARED, shmid, 0);
    if (p == MAP_FAILED)
        ERR_EXIT("mmap");
    
    strcpy(p->name, "Jack");
    p->age = 20;
    printf("write success\n");
    close(shmid);

    return 0;
}