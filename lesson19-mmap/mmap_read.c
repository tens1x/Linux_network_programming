#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

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
    char name[4];
    int age;
}STU;

int main(int argc, char* argv[])
{   
    if (argc != 2){
        fprintf(stderr, "Usage = %s <file> \n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int fd;
    fd = open(argv[1], O_RDWR);
    if(fd == -1)
        ERR_EXIT("open");

    lseek(fd, sizeof(STU) * 5 - 1, SEEK_SET);
    write(fd, "", 1);

    STU *p = (STU*) mmap(NULL, sizeof(STU)*5, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if( p == NULL)
        ERR_EXIT("mmap");

    
    int i ;
    for (i = 0; i < 5; i++){
        printf("name = %s age = %d \n", (p + i)->name, (p + i)->age);
    }

    printf("Initialize finished\n");
    munmap(p, sizeof(STU) * 5);
    printf("exit \n");
    return 0;
}