// 3-close.c

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(void) {

    char filename[] = "test-3-close";
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;    // 644

    int fd = creat(filename, mode);

    if (fd == -1) {
        printf("creat file fail\n");
        return 1;
    }
    else {
        printf("creat file OK\n");
    }

    if (close(fd) == -1) {
        printf("close file fail\n");
        return 2;
    }
    else {
        printf("close file OK\n");
    }

    return 0;
}
