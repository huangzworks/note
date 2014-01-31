// 4-owner-c

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

    // 1) check argument

    if (argc != 2) {
        printf("usage: ./a.out filepath\n");
        return 1;
    }

    // 2) get file info

    struct stat file;
    char *filepath = argv[1];

    if (stat(filepath, &file) == -1) {
        printf("get file %s info fail\n", filepath);
        return 2;
    }

    // 3) print owner info

    printf("file owner uid = %u , gid = %u .\n", file.st_uid, file.st_gid);

    // 4)

    return 0;
}
