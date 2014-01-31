#include <sys/types.h>  // socket
#include <sys/socket.h> // socket
#include <stdio.h>  // printf
#include <stdlib.h> // exit
#include <string.h> // bzero
#include <arpa/inet.h>  // htons, inet_pton

#define MAXLINE (4096)

int
main(int argc, char *argv[])
{
    int sockfd, n;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;

    if (argc != 2) {
        fprintf(stderr, "usage: a.out <IPaddress>\n");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(2);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(13);

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error for %s\n", argv[1]);
        exit(3);
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        exit(4);
    }

    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {

        recvline[n] = '\0';

        if (fputs(recvline, stdout) == EOF) {
            fprintf(stderr, "fputs error\n");
            exit(5);
        }
    }

    if (n < 0) {
        perror("read error\n");
        exit(6);
    }

    exit(0);
}
