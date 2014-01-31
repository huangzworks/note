// 4-print-type.c

#include <sys/stat.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("usage: ./a.out filepath\n");
        return 1;
    }

    char *filepath = argv[1];
    struct stat file;

    if (stat(filepath, &file) == -1) {
        printf("get file %s info fail\n", filepath);
        return 2;
    }

    if (S_ISREG(file.st_mode)) {
        printf("regular\n");
    }
    else if (S_ISDIR(file.st_mode)) {
        printf("directory\n");
    }
    else if (S_ISCHR(file.st_mode)) {
        printf("character device\n");
    }
    else if (S_ISBLK(file.st_mode)) {
        printf("block device\n");
    }
    else if (S_ISFIFO(file.st_mode)) {
        printf("FIFO\n");
    }
    else if (S_ISLNK(file.st_mode)) {
        printf("symbolic link\n");
    }
    else if (S_ISSOCK(file.st_mode)) {
        printf("socket\n");
    }
    else {
        printf("unknow type\n");
    }

    return 0;
}
