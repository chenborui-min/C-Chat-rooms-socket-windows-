#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_
#include<WinSock2.h>//windows的网络库
#pragma comment(lib,"ws2_32.lib")//库文件
#include <iostream>

//#define PORT 8888 //[0,65536) 0-1024系统占用
#define err(errMsg) printf("[line:%d]%s failed code %d",__LINE__,errMsg,WSAGetLastError())

//打开网络库
bool init_socket();
//关闭网络库
bool close_socket();
//创建服务器socket
SOCKET createServerSocket(int PORT);
//创建客户端socket
SOCKET createClientSocket(const char* ip, int PORT);

#endif // ! _TCPSOCKET_H_
