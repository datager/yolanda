#include "lib/common.h"

void send_data(int sockfd) {
    // 准备数组数据
    const int BUF_SIZE = 1024000;
    char *buf;
    buf = malloc(BUF_SIZE + 1);
    for (int i = 0; i < BUF_SIZE; i++) {
        buf[i] = 'a';
    }
    buf[BUF_SIZE] = '\0';

    const char *cp;
    cp = buf;
    size_t remaining = strlen(buf);
    while (remaining) {
        // `send()` 函数用于将数据发送到套接字。它有四个参数：
        // `sockfd`: 套接字描述符，指示要发送数据的目标套接字。
        // `cp`: 指向要发送数据的缓冲区的指针。
        // `remaining`: 要发送的数据的长度。
        // `flags`: 发送标志，通常为0。可以包含不同的标志，例如`MSG_DONTWAIT`（非阻塞发送）等，根据需要设置。
        int n_written = send(sockfd, cp, remaining, 0);
        fprintf(stdout, "send into buffer %ld \n", n_written);
        if (n_written <= 0) {
            error(1, errno, "send failed");
            return;
        }
        remaining -= n_written; // 剩余待发送的长度，减少
        cp += n_written; // 待发送的起始位置，向后偏移
    }
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 2)
        error(1, 0, "usage: tcpclient <IPaddress>");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 赋值 servaddr 地址
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr); // 把ip地址转化为用于网络传输的二进制数值: 函数名的p和n分别代表表达（presentation)和数值（numeric)
    servaddr.sin_port = htons(12345);

    int connect_rt = connect(sockfd, (struct sockaddr *) &servaddr,
                             sizeof(servaddr)); // 用 sockfd 和 servaddr 来 conn() 到 server 做三次握手
    if (connect_rt < 0) { // 返回值 0 表示成功, -1 表示失败
        error(1, errno, "connect failed ");
    }

    send_data(sockfd);
    close(sockfd);
    exit(0);
}

