// 4-permission.c

#include <sys/stat.h>
#include <stdio.h>

#define READ_PERMISSION 4
#define WRITE_PERMISSION 2
#define EXECUTE_PERMISSION 1

#define RWX_PERMISSION 7

int main(int argc, char *argv[]) {
   
    // 1) check argument

    if (argc != 2) {
        printf("usage: ./a.out filepath");
        return 1;
    }

    // 2) get file info

    struct stat file;
    char *filepath = argv[1];

    if (stat(filepath, &file) == -1) {
        printf("get file info fail\n");
        return 2;
    }

    // 3) test permission

    unsigned int owner = 0, group = 0, other = 0;

    // owner test
    if (file.st_mode & S_IRWXU == S_IRWXU) {
        owner = RWX_PERMISSION;
    }
    else {
        if (file.st_mode & S_IRUSR) 
            owner += READ_PERMISSION;

        if (file.st_mode & S_IWUSR)
            owner += WRITE_PERMISSION;

        if (file.st_mode & S_IXUSR)
            owner += EXECUTE_PERMISSION;
    }

    // group test

    if (file.st_mode & S_IRWXG == S_IRWXG) {
        group = RWX_PERMISSION;
    }
    else {
        if (file.st_mode & S_IRGRP)
            group += READ_PERMISSION;

        if (file.st_mode & S_IWGRP)
            group += WRITE_PERMISSION;

        if (file.st_mode & S_IXGRP)
            group += EXECUTE_PERMISSION;
    }

    // other test

    if (file.st_mode & S_IRWXO == S_IRWXO) {
        other = RWX_PERMISSION;
    }
    else {
        if (file.st_mode & S_IROTH)
            other += READ_PERMISSION;

        if (file.st_mode & S_IWOTH)
            other += WRITE_PERMISSION;

        if (file.st_mode & S_IXOTH)
            other += EXECUTE_PERMISSION;
    }

    // 4) print permission

    printf("file %s permission = %u%u%u\n", filepath, owner, group, other);

    return 0;
}
