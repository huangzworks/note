/*
 * 14-1.c
 */
#include "apue.h"
#include <errno.h>
#include <fcntl.h>

char buf [500000];

int
main(void)
{
    int ntowrite, nwrite;
    char *ptr;

    ntowrite = read(STDIN_FILENO, buf, sizeof(buf));
    fprintf(stderr, "read %d bytes\n", ntowrite);

    // 设置非阻塞标志
    set_fl(STDOUT_FILENO, O_NONBLOCK);

    ptr = buf;
    while (ntowrite > 0) {
        errno = 0;
        nwrite = write(STDOUT_FILENO, ptr, ntowrite);
        // 每次都将写出字节数和错误号码打印到标准输出
        fprintf(stderr, "nwrite = %d, errno = %d\n", nwrite, errno);
        
        if (nwrite > 0) {
            ptr += nwrite;
            ntowrite -= nwrite;
        }
    }

    // 清除非阻塞
    clr_fl(STDOUT_FILENO, O_NONBLOCK);

    exit(0);
}
