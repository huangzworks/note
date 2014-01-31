// 3-dup.c

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

void main(void) {
   
    // 1) dup

    int d1 = dup(STDIN_FILENO);

    printf("file descriptor duplicated by dup, number = %d\n", d1);

  
    // 2) dup2

    int newfd = 123;
    int d2 = dup2(STDIN_FILENO, newfd);

    printf("file descriptor duplicated by dup2 with newfd %d, number = %d\n", newfd, d2);


    // 3) dup4

    int newfd_2 = 456;
    int d3= dup3(STDIN_FILENO, newfd_2, O_CLOEXEC);

    printf("file descriptor duplicated by dup3 with newfd %d, number = %d\n", newfd_2, d3);
}
