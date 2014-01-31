// 8-get-exit-status.c

#include <unistd.h>
#include <stdio.h>
#include <wait.h>

int main(void) {

    int status;
    pid_t pid = fork();

    if (pid == -1) {
        printf("fork error\n");
        return 1;
    }

    if (pid != 0) {
        // parent
        wait(&status);
        if (WIFEXITED(status)) {
            printf("child id %d ,normal terminated with status %d .\n", pid, WEXITSTATUS(status));
        }
        else {
            printf("child not terminate normaly, something went wrong\n");
        }
    }
    else {
        // child
        return 2;   // status
    }
}
