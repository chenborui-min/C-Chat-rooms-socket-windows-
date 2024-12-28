#include "tcpsocket.h"
#include <iostream>
#include <ws2tcpip.h> // 包含 inet_pton 的头文件
using namespace std;

bool init_socket()
{
    WSADATA wsadata;
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))//参数一：请求的socket版本 参数二：传出参数
    {
        cout << "WSAStartup failed code <<WSAGetlastError()<<";
        return false;
    }

    return true;
}
/*WSADATA wsadata: 这是一个结构体，用来存储 Windows Sockets 库的信息。
WSAStartup(MAKEWORD(2, 2), &wsadata):
MAKEWORD(2, 2)：指定请求的 Winsock 版本号，2.2 表示版本 2.2。该函数会初始化 Winsock 库。
&wsadata: 这是一个输出参数，返回有关当前 Windows 套接字实现的信息。
WSAStartup 返回 0 表示成功，非零值表示失败。
WSAGetlastError()：返回最近一次 Winsock 错误的错误码。
此函数用于初始化 Windows 套接字库，返回 true 表示初始化成功，false 表示失败。*/
bool close_socket()
{
    if (0 != WSACleanup())
    {
        cout << "WSACleanup failed code <<WSAGetlastError()<<";
        return false;
    }

    return true;
}
/*WSACleanup(): 清理 Winsock 库，释放资源。若返回非零值，表示清理失败。
WSAGetlastError(): 返回最近一次错误的代码。
此函数用于关闭 Winsock 库并释放资源。*/
SOCKET createServerSocket(int PORT)
{
    //创建空的socket
    //af:地址协议族 ipv4 ipv6
    //type：传输协议类型 流式套接字 数据报
    //protocl：使用具体的某个传输协议
    SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == INVALID_SOCKET)
    {
        err("socket");
        return INVALID_SOCKET;
    }//创建失败

    //绑定ip和端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;//ipv4
    addr.sin_port = htons(PORT);//端口,本地字节序转化为网络字节序，大端存储转小端存储
    addr.sin_addr.s_addr = ADDR_ANY;//ip地址,inet_addr("127.0.0.1")

    //绑定
    if (SOCKET_ERROR == bind(fd, (struct sockaddr*)&addr, sizeof(addr)))
    {
        err("bind");
        closesocket(fd); // 绑定失败时关闭 socket
        return INVALID_SOCKET;
    }

    //监听信息
    if (listen(fd, 10) == SOCKET_ERROR)
    {
        err("listen");
        closesocket(fd); // 监听失败时关闭 socket
        return INVALID_SOCKET;
    }
    return fd;
}
/*SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP): 创建一个流式套接字，使用 TCP 协议。

AF_INET: 指定使用 IPv4 地址。
SOCK_STREAM: 指定套接字类型为流式套接字，表示使用 TCP 协议。
IPPROTO_TCP: 指定使用 TCP 协议。
INVALID_SOCKET: 这是 socket() 函数调用失败时返回的值，表示套接字创建失败。

struct sockaddr_in addr: 这是一个结构体，表示 IPv4 地址的格式。它包含以下字段：

sin_family = AF_INET: 指定地址族为 IPv4。
sin_port = htons(PORT): 端口号，通过 htons 函数将主机字节序转为网络字节序（大端存储）。
sin_addr.s_addr = ADDR_ANY: 表示绑定到所有可用的网络接口，ADDR_ANY 是指允许任何地址连接。
bind(fd, (struct sockaddr*)&addr, sizeof(addr)): 绑定套接字 fd 到指定的地址和端口。

fd 是之前创建的套接字描述符。
(struct sockaddr*)&addr 是将 sockaddr_in 类型转换为 sockaddr 类型。
sizeof(addr) 是地址结构的大小。
listen(fd, 10): 将套接字 fd 设置为监听模式，10 表示最多允许 10 个客户端请求排队等待。

closesocket(fd): 关闭套接字。

该函数创建一个 TCP 服务器套接字，绑定到指定端口，设置为监听模式，返回套接字描述符，如果出现错误，返回 INVALID_SOCKET。*/
SOCKET createClientSocket(const char* ip,int PORT)
{
    //创建空的socket
    //af:地址协议族 ipv4 ipv6
    //type：传输协议类型 流式套接字 数据报
    //protocl：使用具体的某个传输协议
    SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == INVALID_SOCKET)
    {
        err("socket");
        return INVALID_SOCKET;
    }//创建失败

    //与服务器建立连接
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;//ipv4
    addr.sin_port = htons(PORT);//端口,本地字节序转化为网络字节序，大端存储转小端存储
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0)
    {
        err("inet_pton");
        closesocket(fd); // 解析 IP 地址失败时关闭 socket
        return INVALID_SOCKET;
    }//ip地址,inet_addr("127.0.0.1")
    if (INVALID_SOCKET == connect(fd, (struct sockaddr*)&addr, sizeof(addr)))
    {
        err("connect");
        closesocket(fd); // 连接失败时关闭 socket
        return INVALID_SOCKET;
    }
    /*SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP): 创建一个流式套接字，使用 TCP 协议。参数解释与 createServerSocket 中相同。

struct sockaddr_in addr: 与服务器端相同，用于存储服务器的 IP 地址和端口号。

inet_pton(AF_INET, ip, &addr.sin_addr): 将字符串形式的 IP 地址（例如 "127.0.0.1"）转换为网络字节序的二进制格式，存储在 addr.sin_addr 中。

AF_INET: 表示 IPv4 地址族。
ip: 传入的字符串格式的 IP 地址。
&addr.sin_addr: 存储转换后的地址。
connect(fd, (struct sockaddr*)&addr, sizeof(addr)): 与指定的服务器建立连接。

fd 是客户端套接字。
(struct sockaddr*)&addr 是服务器的地址信息。
sizeof(addr) 是地址结构的大小。
closesocket(fd): 关闭套接字。

该函数用于创建一个 TCP 客户端套接字，连接到指定的服务器和端口。如果连接失败，返回 INVALID_SOCKET。*/


    return fd;
}
/*init_socket() 用于初始化 Winsock 库。
close_socket() 用于清理 Winsock 库。
createServerSocket(int PORT) 用于创建一个 TCP 服务器套接字，绑定到指定端口并监听客户端连接。
createClientSocket(const char* ip, int PORT) 用于创建一个 TCP 客户端套接字，连接到指定的服务器 IP 和端口。
函数使用了 SOCKET 类型来表示套接字，通过 Winsock 提供的 API 来处理套接字的创建、绑定、监听、连接等操作。*/
/*SOCKET socket(int af, int type, int protocol);
    af：地址族（Address Family）
    AF_INET：IPv4 地址。
    AF_INET6：IPv6 地址。
    AF_UNIX：Unix 域套接字。
    type：套接字类型（Socket Type）
    SOCK_STREAM：流式套接字，通常用于 TCP 连接。
    SOCK_DGRAM：数据报套接字，通常用于 UDP。
    protocol：协议类型，通常选择 0，操作系统会自动选择合适的协议。
    IPPROTO_TCP：使用 TCP 协议。
    IPPROTO_UDP：使用 UDP 协议。
    返回值：
    成功：返回一个套接字描述符（SOCKET 类型）。
    失败：返回 INVALID_SOCKET，需要调用 WSAGetLastError() 获取具体错误信息。
int bind(SOCKET s, const struct sockaddr *addr, int addrlen);
    s：待绑定的套接字。
    addr：指向 sockaddr 结构的指针，指定本地地址和端口。
    addrlen：sockaddr 结构的大小，通常使用 sizeof(sockaddr_in)。
    返回值：
    成功：返回 0。
    失败：返回 SOCKET_ERROR，需要调用 WSAGetLastError() 获取错误代码。
int listen(SOCKET s, int backlog);
    s：待监听的套接字。
    backlog：指定系统内核在等待连接队列中的最大连接数。通常设置为 5 或 10。
    成功：返回 0。
    失败：返回 SOCKET_ERROR，需要调用 WSAGetLastError() 获取错误代码。
SOCKET accept(SOCKET s, struct sockaddr *addr, int *addrlen);
    s：监听套接字。
    addr：指向 sockaddr 结构的指针，用于返回客户端的地址信息。
    addrlen：指向整数的指针，返回地址结构的大小（通常是 sizeof(sockaddr_in)）。
    成功：返回一个新的套接字描述符，用于与客户端进行通信。
    失败：返回 INVALID_SOCKET，需要调用 WSAGetLastError() 获取错误代码。
int connect(SOCKET s, const struct sockaddr *name, int namelen);
    s：待连接的套接字。
    name：指向 sockaddr 结构的指针，包含服务器的 IP 地址和端口号。
    namelen：sockaddr 结构的大小，通常使用 sizeof(sockaddr_in)。
    成功：返回 0。
    失败：返回 SOCKET_ERROR，需要调用 WSAGetLastError() 获取错误代码。
int send(SOCKET s, const char *buf, int len, int flags);
    s：套接字描述符。
    buf：指向要发送的数据缓冲区。
    len：要发送的数据的字节数。
    flags：标志位（通常设为 0，也可以设置为 MSG_DONTROUTE 等）。
    成功：返回发送的字节数。
    失败：返回 SOCKET_ERROR，需要调用 WSAGetLastError() 获取错误代码。
int recv(SOCKET s, char *buf, int len, int flags);
    s：套接字描述符。
    buf：指向接收数据的缓冲区。
    len：缓冲区的大小。
    flags：标志位（通常设为 0，也可以设置为 MSG_WAITALL 等）。
    成功：返回接收到的字节数。
    失败：返回 SOCKET_ERROR，需要调用 WSAGetLastError() 获取错误代码。
    连接关闭：返回 0。
*/