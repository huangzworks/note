// 8-data-duplicate.c

#include <unistd.h>
#include <stdio.h>

int global = 0;

int main(void) {
   
    int local = 0;

    printf("init global = %d , local = %d\n", global, local);

    pid_t child_pid = fork();

    if (child_pid == 0) {
        global++;
        local++;

        printf("child running, global = %d , local = %d\n", global, local);
    }
    else if (child_pid > 0) {
        global++;
        local++;

        printf("parent running, global = %d , local = %d\n", global, local);
    }
    else {
        printf("Fork error\n");
    }

    return 0;
}
