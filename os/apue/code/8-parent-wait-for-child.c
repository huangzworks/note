// 8-parent-wait-for-child.c

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main(void) {
    
    pid_t pid = fork();

    if (pid == 0) {
        printf("child return\n");
    } 
    else if (pid > 0) {
        wait(NULL);
        printf("parent return\n");
    }
    else {
        printf("fork error\n");
        return 1;
    }
}
