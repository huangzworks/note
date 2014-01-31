#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define LISTENQ (1024)
#define BUFSIZE (1024)

#define SERVER_PORT (52350)

void echo_ip_and_port(char *endpoint, struct sockaddr_in *addr);

int main(void)
{
    int listenfd, client_fd;
    socklen_t len;
    struct sockaddr_in addr, local_addr, client_addr;

    // 创建套接字
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    // 创建地址
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERVER_PORT);

    // 绑定地址
    bind(listenfd, (struct sockaddr *)&addr, sizeof(addr));

    // 监听连接
    listen(listenfd, LISTENQ);

    // 接收连接
    client_fd = accept(listenfd, NULL, NULL);

    /*
     * 获取并打印服务器 ip 和端口
     */
    memset(&local_addr, 0, sizeof(local_addr));
    len = sizeof(local_addr);
    getsockname(listenfd, (struct sockaddr *)&local_addr, &len);

    echo_ip_and_port("Server", &local_addr);

    /*
     * 获取并打印客户端 ip 和端口
     */
    memset(&client_addr, 0, sizeof(client_addr));
    len = sizeof(client_addr);
    getpeername(client_fd, (struct sockaddr *)&client_addr, &len);

    echo_ip_and_port("Client", &client_addr);

    // 退出
    exit(0);
}

void echo_ip_and_port(char *endpoint, struct sockaddr_in *addr) {

    char buf[BUFSIZE];

    printf("%s ip is %s , port is %d .\n",
           endpoint,
           inet_ntop(addr->sin_family, &addr->sin_addr, buf, INET_ADDRSTRLEN),
           ntohs(addr->sin_port));
}
