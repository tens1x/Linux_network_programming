#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>

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

    int shmid = shm_open("/xyz", O_CREAT | O_RDWR, 0666);
    if(shmid == -1)
        ERR_EXIT("shm_open");

    printf("shm open success \n");

    if(ftruncate(shmid, sizeof(STU)) == -1)//修改共享内存大小
        ERR_EXIT("ftruncate\n");

    struct stat st;
    if(fstat(shmid, &st) == -1)//获取消息队列状态
        ERR_EXIT("fstat \n");
      
    printf("size = %ld , mode = %o \n", st.st_size, st.st_mode & 07777);
    close(shmid);

    return 0;
}