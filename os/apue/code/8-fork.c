// 8-fork.c

#include <unistd.h>
#include <stdio.h>

int main(void) {

    pid_t child_id;

    child_id = fork();
    
    if (child_id == 0) {
        printf("Child process running, id = %d\n", getpid());
        printf("Child process's parent id = %d\n", getppid());
    }
    else if (child_id > 0) {
        printf("Parent process running, id = %d\n", getpid());
    }
    else {
        // child_id == -1
        printf("Fork error\n");
    }

    return 0;
}
