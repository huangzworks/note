// 3-hole.c

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>

#define FIRST_CONTENT "abcdefg"
#define SECOND_CONTENT "hello moto"

#define HOLE_LENGTH 256

int main(void) {

    // 1) creat file

    char filename[] = "test-3-hole";
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;    // 644

    int fd = creat(filename, mode);

    if (fd == -1) {
        printf("creat file fail\n");
        return 1;
    }

    // 2) write first content

    if (write(fd, FIRST_CONTENT, sizeof(FIRST_CONTENT)) != sizeof(FIRST_CONTENT)) {
        printf("write first content fail\n");
        return 2;
    }

    // 3) make hole

    lseek(fd, HOLE_LENGTH, SEEK_CUR);

    // 4) write second content

    if (write(fd, SECOND_CONTENT, sizeof(SECOND_CONTENT)) != sizeof(SECOND_CONTENT)) {
        printf("write second content fail\n");
        return 2;
    }

    return 0;
}
