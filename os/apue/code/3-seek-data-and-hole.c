// 3-seek-data-and-hole.c

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {

    // 1) check filename input

    if (argc != 2) {
        printf("usage: ./out filename\n");
        return 1;
    }

    // 2) open file for read

    char *filename = argv[1];
    int fd = open(filename, O_RDONLY);

    if (fd == -1) {
        printf("open file fail\n");
        return 2;
    }

    // 3) seek to first data

    off_t data_offset;

    data_offset = lseek(fd, 0, SEEK_DATA);
    if (data_offset == -1) {
        printf("get data offset fail\n");
        return 4;
    }

    printf("first data offset of file %s is %lld\n", filename, (long long)data_offset);

    // 4) seek to first hole

    off_t hole_offset;

    hole_offset = lseek(fd, 0, SEEK_HOLE);
    if (hole_offset == -1) {
        printf("get hole offset fail\n");
        return 5;
    }

    printf("first hole offset of file %s is %lld\n", filename, (long long)hole_offset);

    return 0;
}
