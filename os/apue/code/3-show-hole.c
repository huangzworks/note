// 3-show-hole.c

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FIRST_CONTENT "abcdefg"
#define SECOND_CONTENT "hello moto"

#define HOLE_LENGTH 256

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {

    // 1) open file

    if (argc != 2) {
        printf("usage: ./out file-with-hole\n");
        return 1;
    }

    char *filename = argv[1];
    int fd = open(filename, O_RDONLY);
    
    if (fd == -1) {
        printf("open file fail\n");
        return 2;
    }

    // 2) iterate file

    int read_status;
    char buf[BUF_SIZE];
    off_t current_offset, data_offset, hole_offset;

    while (1) {

        read_status = read(fd, buf, BUF_SIZE);
        
        if (read_status == -1) {
            printf("read error\n");
            return 3;
        }

        if (read_status == 0) {
            printf("EOF\n");
            return 0;
        }

        // inspect buf to decide what to seek next
        if (buf[0] == '\0') {
            // current offset pointing a hole
            // seek to next data 
            current_offset = lseek(fd, 0, SEEK_CUR);
            if (current_offset == -1) {
                printf("get offset fail\n");
                return 4;
            }

            data_offset = lseek(fd, current_offset, SEEK_DATA);
            if (data_offset == -1) {
                printf("data seek fail\n");
                return 4;
            }

            printf("data start from offset %ld\n", data_offset);
        }
        else {
            // current offset pointing data
            // seek to next hole
            current_offset = lseek(fd, 0, SEEK_CUR);
            if (current_offset == -1) {
                printf("get offset fail\n");
                return 5;
            }

            hole_offset = lseek(fd, current_offset, SEEK_HOLE);
            if (hole_offset == -1) {
                printf("hole seek fail\n");
                return 5;
            }

            printf("hole start from offset %ld\n", hole_offset);
        }
    
    }
}
