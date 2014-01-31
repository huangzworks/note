// 3-pread.c

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUF_SIZE 1024

#define READ_SIZE 10

off_t get_current_offset(int fd);

int main(int argc, char *argv[]) {

    // 1) check argument

    if (argc != 2) {
        printf("usage: ./a.out file\n");
        return 1;
    }


    // 2) open file

    char *filename = argv[1];

    int fd = open(filename, O_RDONLY);

    if (fd == -1) {
        printf("open file %s fail\n", filename);
        return 2;
    }


    // 3) read

    off_t before_read_offset = get_current_offset(fd);

    char buf[BUF_SIZE];

    if (read(fd, buf, READ_SIZE) == -1) {
        printf("read fail\n");
        return 3;
    }

    off_t after_read_offset = get_current_offset(fd);

    printf("after read %d bytes from file, offset move from %lld to %lld\n",
           READ_SIZE,
           (long long)before_read_offset,
           (long long)after_read_offset);


    // 4) pread

    off_t before_pread_offset = get_current_offset(fd);

    if (pread(fd, buf, READ_SIZE, before_pread_offset) == -1) {
        printf("pread fail\n");
        return 4;
    }

    off_t after_pread_offset = get_current_offset(fd);

    printf("after pread %d bytes from file, offset move from %lld to %lld\n",
           READ_SIZE,
           (long long)before_pread_offset,
           (long long)after_pread_offset);

    return 0;
}

off_t get_current_offset(int fd) {
    return lseek(fd, 0, SEEK_CUR);
}
