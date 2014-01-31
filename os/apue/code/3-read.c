// 3-read.c

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define CONTENT "hello moto"
#define LEN_OF_CONTENT sizeof(CONTENT)

int main(void) {

    // 1) create file

    char filename[] = "test-3-read";
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;    // 644

    int fd = creat(filename, mode);

    if (fd == -1) {
        printf("creat file fail\n");
        return 1;
    } 

   
    // 2) write file

    if (write(fd, CONTENT, LEN_OF_CONTENT) != LEN_OF_CONTENT) {
        printf("write error\n");
        return 2;
    }

    close(fd);


    // 3) reopen and read

    fd = open(filename, O_RDONLY);

    if (fd == -1) {
        printf("open file fail\n");
        return 3;
    }

    char buf[LEN_OF_CONTENT];

    if (read(fd, buf, LEN_OF_CONTENT) != LEN_OF_CONTENT) {
        printf("read file fail\n");
        return 4;
    }

    
    // 4) print content

    printf("file content: %s\n", buf);

    return 0;
}
