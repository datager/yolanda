#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void print_sockaddr_in(struct sockaddr_in addr)
{
    // 使用 inet_ntoa 函数将 IP 地址转换为字符串
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip_str, INET_ADDRSTRLEN);

    // 使用 ntohs 函数将端口号从网络字节顺序转换为主机字节顺序
    unsigned short port = ntohs(addr.sin_port);

    printf("IP Address: %s\n", ip_str);
    printf("Port: %d\n", port);
}

int make_socket(uint16_t port)
{
    /* 创建 socket 句柄, 文档见 https://pubs.opengroup.org/onlinepubs/009696699/functions/socket.html */
    // domain = PF_INET 表示 IPV4
    // type = SOCK_STREAM 表示 TCP
    // protocal = 0 这个字段已废弃, 直接传 0 即可
    int sockfd = socket(PF_INET, SOCK_STREAM, 0); // 返回的 sock 是数字，如 3，表示文件描述符 fd
    if (sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* 将 socket 句柄 bind() 到 ip 和 port 上 */
    struct sockaddr_in name;   // sockaddr_in 是 ipv4 类型socket 的地址, sockaddr_in6 是 ipv6 类型socket 的地址
    name.sin_family = AF_INET; // family = AF_INET 表示 IPv4 类型
    // INADDR_ANY 就是 0.0.0.0, 意为监听本机所有的 ip 地址（如本机有多个网卡）
    // 也可以写成 name.sin_addr.s_addr = inet_addr("192.168.1.1"); 其中 inet_addr() 函数将点分十进制的 IP 地址转换为 in_addr_t 类型的网络字节顺序
    name.sin_addr.s_addr = htonl(INADDR_ANY); // htonl 将主机字节顺序（通常是小端字节序）转换为网络字节顺序（大端字节序），即"host to network long"。这在网络编程中很常见，因为网络协议通常使用大端字节序来传输数据
    name.sin_port = htons(port);              // htons 将主机字节顺序（通常是小端字节序）转换为网络字节顺序（大端字节序），即"host to network short"。这在网络编程中很常见，因为网络协议通常使用大端字节序来传输数据。

    if (bind(sockfd, (struct sockaddr *)&name, sizeof(name)) < 0) // 把IPV4地址转换成通用地址格式，同时传递长度
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    /* 只打印日志：打印创建的 socket、IP 地址和端口信息 */
    printf("sockfd created: %d, pid: %d\n", sockfd, getpid());

    print_sockaddr_in(name);

    return sockfd;
}

/*
// https://github.com/datager/yolanda/blob/master/chap-4/make_socket.c
// code result:
socked created: 3
IP Address: 0.0.0.0
Port: 12345

// 这样刚创建 socket，但未建立连接的话，socket 状态是 CLOSED，可通过查看进程的 fd 看到如下：
# lsof -np 13296
a.out   13296    root    3u  IPv4 0x4ea0835023154a7f      0t0      TCP *:italk (CLOSED)
*/
int main(int argc, char **argv)
{
    int sockfd = make_socket(12345);
    sleep(1000);
    // exit(0);
}
