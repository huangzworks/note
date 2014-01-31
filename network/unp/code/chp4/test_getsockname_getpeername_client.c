#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define SERVER_PORT (52350)

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in addr;

    if (argc != 2) {
        fprintf(stderr, "usage: a.out <ddd.ddd.ddd.ddd>\n");
        exit(1);
    }

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 创建地址
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &addr.sin_addr);
    addr.sin_port = htons(SERVER_PORT);

    // 连接服务器
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect error");
        exit(2);
    }

    printf("connect ok\n");

    exit(0);
}
