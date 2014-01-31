// 4-set-x-permission.c

#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
   
    // 

    if (argc != 2) {
        fprintf(stderr, "usage: ./a.out filepath\n");
        return 1;
    }

    // 

    char *filepath = argv[1];
    struct stat file;

    if (stat(filepath, &file) == -1) {
        perror("Get file info fail\n");
        return 2;
    }

    if (chmod(filepath, file.st_mode | S_IXUSR) == -1) {
        perror("Set owner execute permission fail");
        return 3;
    }

    return 0;
}
