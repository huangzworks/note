// 4-change-owner.c

#include <unistd.h> // chown
#include <stdio.h>  // printf, perror

#define ROOT_UID 0
#define ROOT_GID 0

int main(int argc, char *argv[]) {

    // 1)

    if (argc != 2) {
        printf("usage: ./a.out filepath");
        return 1;
    }

    // 2)

    char *filepath = argv[1];

    if (chown(filepath, ROOT_UID, ROOT_GID) == -1) {
        perror("Change owner fail");
        return 2;
    }

    // 3)

    return 0;
}
