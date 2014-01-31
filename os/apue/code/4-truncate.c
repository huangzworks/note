// 4-truncate.c

#include <unistd.h> // truncate
#include <stdio.h>  // stderr, fprintf, perror
#include <stdlib.h> // atoll

int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "usage: ./a.out filepath new_size\n");
        return 1;
    }

    char *filepath = argv[1];
    long long int new_size = atoll(argv[2]);

    if (truncate(filepath, new_size) == -1) {
        perror("Truncate fail");
        return 2;
    }

    return 0;
}
