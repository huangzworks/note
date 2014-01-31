// 4-printf-type-2.c

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

    switch (file.st_mode & S_IFMT) {
        case S_IFREG:
            printf("regular\n");
            break;
        case S_IFDIR:
            printf("directory\n");
            break;
        case S_IFCHR:
            printf("character device\n");
            break;
        case S_IFBLK:
            printf("block device\n");
            break;
        case S_IFIFO:
            printf("FIFO\n");
            break;
        case S_IFLNK:
            printf("symbolic link\n");
            break;
        case S_IFSOCK:
            printf("socket\n");
            break;
        default:
            printf("unknow type\n");
    }

    return 0;
}

