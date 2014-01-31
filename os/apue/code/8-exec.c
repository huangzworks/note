// 8-exec.c

#include <unistd.h>
#include <stdio.h>

int main(void) {

    pid_t pid = fork();

    if (pid == -1) {
        printf("fork error\n");
        return 1;
    }
    else if (pid == 0) {
        // child
        execl("hello_world.out", (char *)NULL);
    }

    return 0;
}
