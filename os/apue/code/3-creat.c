// 3-creat.c

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

int main(void) {

    char filename[] = "test-3-creat";

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;    // 644

    if (creat(filename, mode) == -1) {
        printf("creat file fail\n");
        return 1;
    } 
    else {
        printf("creat file OK\n");
        return 0;
    }
}
