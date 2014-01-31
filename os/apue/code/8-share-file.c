// 8-share-file.c

#include <stdio.h>
#include <unistd.h>

#define PARENT_CONENT "parent write\n"
#define CHILD_CONTENT "child write\n"

int main(void) {

    // file

    FILE *f = fopen("8-share-file-text", "wa");

    if (f == NULL) {
        printf("open/create file fail\n");
        return 1;
    }

    // process

    pid_t pid = fork();

    if (pid == 0) {
        // parent
        fputs(PARENT_CONENT, f);
    }
    else if (pid > 0) {
        // child
        fputs(CHILD_CONTENT, f);
    }
    else {
        printf("fork fail\n");
        return 2;
    }

    return 0;
}
