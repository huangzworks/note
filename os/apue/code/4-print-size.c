// 4-print-size.c

#include <stdio.h>      // perror, fprintf, printf
#include <sys/stat.h>   // lstat, struct stat, S_ISREG, S_ISLNK

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: ./a.out filepath\n");
        return 1;
    }

    char *filepath = argv[1];
    struct stat file;

    // use lstat() instead of stat()
    // not follow if filepath is a symbolic link
    if (lstat(filepath, &file) == -1) {
        perror("Get file info fail");
        return 2;
    }

    if (S_ISREG(file.st_mode) || S_ISLNK(file.st_mode))
        printf("%s length = %lld bytes .\n", filepath, (long long)file.st_size);
    else 
        printf("%s is not regular file or symbolic link , length unknow.\n", filepath);
        
    return 0;
}
