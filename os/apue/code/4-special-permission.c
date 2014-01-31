// 4-special-permission.c

#include <stdio.h>
#include <sys/stat.h>

void print_set_or_not(char *bit_name, int is_set);

int main(int argc, char *argv[]) {

    // check argument

    if (argc != 2) {
        fprintf(stderr, "usage: ./a.out filepath\n");
        return 1;
    }

    // get file info

    char *filepath = argv[1];
    struct stat file;

    if (stat(filepath, &file) == -1) {
        perror("Get file info fail");
        return 2;
    }

    // check regular file

    if (S_ISREG(file.st_mode)) {

        print_set_or_not(
            "set-user-ID",
            file.st_mode & S_ISUID == S_ISUID
        );

        print_set_or_not(
            "set-group-ID", 
            file.st_mode & S_ISGID == S_ISGID
        );
    }

    // check directory

    if (S_ISDIR(file.st_mode)) {

        print_set_or_not(
            "sticky",
            file.st_mode & S_ISVTX == S_ISVTX
        );
    }

    return 0;
}

void print_set_or_not(char *bit_name, int is_set) {

    printf("%s bit is ", bit_name);

    if (is_set) 
        printf("set");
    else
        printf("not set");

    printf("\n");
}
