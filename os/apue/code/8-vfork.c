// 8-vfork.c

#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    
    pid_t pid = vfork();

    if (pid == 0) {
        printf("child running\n");
        exit(0); 
        // signal parent to continue ...
    }
    else if (pid > 0) {
        printf("parent running\n");
    }
    else {
        printf("vfork error\n");
        return 1;
    }

    return 0;
}
