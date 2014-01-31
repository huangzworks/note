// 3-print-file-descriptor.c

#include <fcntl.h>
#include <stdio.h>

int main(void) {

    char filename[] = "test-3-print-file-descriptor";

    int flag = O_RDWR | O_CREAT;

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;    // 644

    int fd = open(filename, flag, mode);
    if (fd == -1) {
        printf("open/create file fail\n");
        return 1;
    }
    else {
        printf("fd = %d\n", fd);
        return 0;
    }
}
