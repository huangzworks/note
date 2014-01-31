// 8-ids.c

#include <unistd.h>
#include <stdio.h>

int main(void) {
    
    printf("Parent pid = %d\n", getppid());

    printf("Uid = %d\n", getuid());

    printf("Euid = %d\n", geteuid());

    printf("Gid = %d\n", getgid());

    printf("Egid = %d\n", getegid());
}
