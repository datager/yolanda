#include "lib/common.h"

size_t readn(int sockfd, void *buf, size_t size) {
    char *buf_ptr = buf;
    int remain_size = size; // 还剩余的长度，初始值为 size

    while (remain_size > 0) {
        int r = read(sockfd, buf_ptr, remain_size);
        if (r < 0) { // 返回负数, 表示出错
            if (errno ==
                EINTR) {        // `EINTR` 是一个在C语言中errno.h头文件中定义的错误码，表示"Interrupted system call"，即系统调用被信号中断。这里当`read()`系统调用被信号中断时，可以通过检查`errno`是否为`EINTR`来判断是否需要重新尝试该系统调用。
                printf("got EINTR, continue\n");// 考虑非阻塞的情况，这里需要再次调用read
                continue;
            } else {
                printf("got err: %d, return\n", errno);
                break;
            }
        } else if (r == 0) {
            printf("得到 EOF, 表示套接字关闭, so return\n");
            break;
        }

        remain_size -= r;
        buf_ptr += r;
    }
    return (size - remain_size);// 返回的是实际读取的 byte 数
}

void read_data(int sockfd) {
    char buf[1024];

    int times = 0;
    for (;;) {
        // 因为 socketfd 遵循一切皆文件, 所以和读文件是一样的用法
        // `readn()` 函数返回的0表示从套接字`sockfd`中读取数据时遇到了EOF（文件结束符），即对端已经关闭了连接。当`readn()`返回0时，意味着没有更多的数据可供读取，通常表示连接已经结束，需要关闭相应的套接字并进行清理工作。
        // EOF（End of File）是一个特殊的控制字符，通常用于表示文件结束或输入流的结束。在文本文件中，EOF表示文件的结尾，而在输入流中，EOF表示输入的结束。EOF并不是一个可见字符，而是一个控制信号，用于指示文件或输入的结束。
        // 从 sockfd 读 1024 byte, 到 buf 数组中. 每次只读取 1024 byte，然后循环读取
        ssize_t n = readn(sockfd, buf, 1024);
        if (n == 0) {
            printf("readn got EOF, then quit\n");
            return;
        }
        times++;
        printf("1K read success, times %d\n", times);
        usleep(1000);
    }
}

int main(int argc, char **argv) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("listenfd: %d\n", listenfd);

    // 给 servaddr 赋值
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr)); // bzero() 函数将指定内存区域的内容全部设置为0。将 `servaddr` 结构体清零，以确保其所有字段都被初始化为0值，然后才进行后续的设置。
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(12345);

    // 将 listenfd 与之 bind()
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    // listen 的 backlog为 1024, 是未完成连接队列的大小，这个参数的大小决定了可以接收的并发数目
    int r = listen(listenfd, 1024);
    if (r < 0) {
        printf("listen fail\n");
        exit(r);
    }

    // 循环处理用户请求
    for (;;) {
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        int connfd = accept(listenfd, (struct sockaddr *) &cliaddr,
                            &clilen); // 传入 listenfd, 当且仅当收到请求且会三次握手成功后， 才会返回 connfd, cliaddr 和 clilen
        read_data(connfd); // 读取数据
        close(connfd);// 关闭连接套接字才会释放 socketfd，注意不是监听套接字
    }
}