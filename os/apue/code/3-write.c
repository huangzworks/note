// 3-write.c

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#define CONTENT "hello moto\n"
#define LEN_OF_CONTENT sizeof(CONTENT)

int main(void) {
    
    char filename[] = "test-3-write";
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;    // 644

    int fd = creat(filename, mode);

    if (fd == -1) {
        printf("creat file FAIL\n");
        return 1;
    }

    int write_byte = write(fd, CONTENT, LEN_OF_CONTENT);

    if (write_byte != LEN_OF_CONTENT) {
        printf("write file FAIL\n");
        return 2;
    }

    return 0;
}
