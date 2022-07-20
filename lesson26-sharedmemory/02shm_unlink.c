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

    if(shm_unlink("/xyz") == -1)
        ERR_EXIT("shm_unlink");
    printf("shm unlink success \n");

    return 0;
}