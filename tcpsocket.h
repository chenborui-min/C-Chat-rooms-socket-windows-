#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_
#include<WinSock2.h>//windows�������
#pragma comment(lib,"ws2_32.lib")//���ļ�
#include <iostream>

//#define PORT 8888 //[0,65536) 0-1024ϵͳռ��
#define err(errMsg) printf("[line:%d]%s failed code %d",__LINE__,errMsg,WSAGetLastError())

//�������
bool init_socket();
//�ر������
bool close_socket();
//����������socket
SOCKET createServerSocket(int PORT);
//�����ͻ���socket
SOCKET createClientSocket(const char* ip, int PORT);

#endif // ! _TCPSOCKET_H_
