// 3-lseek.c

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define CONTENT "hello moto"
#define LEN_OF_CONTENT sizeof(CONTENT)

int main(void) {

    // 1) creat file

    char filename[] = "test-3-lseek";
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;    // 644

    // use open instead of creat for read and write
    int fd = open(filename, O_RDWR, mode);

    if (fd == -1) {
        printf("creat file fail\n");
        return 1;
    }

    // 2) write file

    if (write(fd, CONTENT, LEN_OF_CONTENT) != LEN_OF_CONTENT) {
        printf("write error\n");
        return 2;
    }

    // 3) set offset to 0

    if (lseek(fd, 0, SEEK_SET) == -1) {
        printf("lseek error\n");
        return 3;
    }

    // 4) read and print conent

    char buf[LEN_OF_CONTENT];

    if (read(fd, buf, LEN_OF_CONTENT) != LEN_OF_CONTENT) {
        printf("read error\n");
        return 4;
    }

    printf("file conent: %s\n", buf);

    return 0;
}
