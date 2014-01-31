// 3-open-create-with-excl.c

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

int main(void) {

    char filename[] = "test-3-open-create-with-excl.txt";

    // create for read and write
    // and make sure this call create the file
    int flag = O_RDWR | O_CREAT | O_EXCL;    

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;    // 644

    if (open(filename, flag, mode) == -1) {
        if (errno == EEXIST) {
            printf("open fail cause file already exists.\n");
        } else {
            printf("open/create file fail\n");
            printf("errno = %d\n", errno);
        }
        return 1;
    }

    return 0;
}
