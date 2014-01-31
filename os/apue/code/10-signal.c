// 10-signal.c

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void sig_usr(int);

int
main(void) {

    if (signal(SIGUSR1, sig_usr) == SIG_ERR) {
        printf("set SIGUSR1 handler fail\n");
        return 1;
    }

    for ( ; ; )
        pause();
}

void 
sig_usr(int signo)
{
    printf("receive signal %d \n", SIGUSR1);
    printf("bye bye ~\n");
    exit(0);
}
