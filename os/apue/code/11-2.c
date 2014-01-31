// 11-2.c

#include "apue.h"
#include <pthread.h>

void *
thr_fn1(void *arg)
{
    printf("thread 1 returning\n");
    return ((void *) 0);
}

void *
thr_fn2(void *arg)
{
    printf("thread 2 exiting\n");
    pthread_exit((void *) 2);
}

int 
main(void)
{
    int err;
    pthread_t tid1, tid2;
    void *tret;

    // 创建线程 1
    err = pthread_create(&tid1, NULL, thr_fn1, NULL);
    if (err != 0)
        err_quit("create thread 1 fail\n", strerror(err));

    // 创建线程 2
    err = pthread_create(&tid2, NULL, thr_fn2, NULL);
    if (err != 0)
        err_quit("create thread 2 fail\n", strerror(err));

    // 等待线程 1 执行完毕
    err = pthread_join(tid1, &tret);
    if (err != 0)
        err_quit("join thread 1 fail\n", strerror(err));
    printf("thread 1 exit code %d\n", (int) tret);

    // 等待线程 2 执行完毕
    err = pthread_join(tid2, &tret);
    if (err != 0)
        err_quit("join thread 2 fail\n", strerror(err));
    printf("thread 2 exit code %d\n", (int) tret);

    exit(0);
}
