#include <stdio.h>  // printf
#include <stdlib.h> // exit, malloc
#include <arpa/inet.h>  // inet_ntop, inet_pton
#include <netinet/in.h> // struct sockaddr_in, AF_INET
#include <string.h> // strlen

int 
main(int argc, char *argv[])
{
    char *buf;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));

    if (argc != 2) {
        fprintf(stderr, "usage: a.out <ddd.ddd.ddd.ddd>\n");
        exit(1);
    }

    // 将 ip 转换为地址

    if (inet_pton(AF_INET, argv[1], &addr.sin_addr) == -1) {
        perror("inet_pton error");
        exit(2);
    }

    // 将地址转换为 ip

    buf = malloc(strlen(argv[1]) + 1);

    if (inet_ntop(AF_INET, &addr.sin_addr, buf, INET_ADDRSTRLEN) == NULL) {
        perror("inet_ntop error");
        exit(3);
    }

    // 打印 ip

    printf("The input ip is: %s\n", buf);

    free(buf);

    return 0;
}
