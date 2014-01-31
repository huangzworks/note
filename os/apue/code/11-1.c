// 11-1.c

#include "apue.h"
#include <pthread.h>

pthread_t ntid;

// 打印进程 id 和线程 id
void
printids(const char *s)
{
    pid_t pid = getpid();

    pthread_t tid = pthread_self();

    printf("%s pid %u tid %u (0x%x)\n",
           s, (unsigned int) pid, (unsigned int) tid, (unsigned int) tid);
}

// 新线程的执行函数
void *
thr_fn(void *arg)
{
    printids("new thread: ");
    return ((void *)0);
}

int
main(void)
{
    int err = pthread_create(&ntid, NULL, thr_fn, NULL);
    if (err != 0)
        err_quit("can't create thread: %s\n", strerror(err));

    printids("main thread:");

    // 等待子线程完成
    sleep(1);

    exit(0);
}
