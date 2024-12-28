#include "tcpsocket.h"
#include <iostream>
#include <ws2tcpip.h> // ���� inet_pton ��ͷ�ļ�
using namespace std;

bool init_socket()
{
    WSADATA wsadata;
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))//����һ�������socket�汾 ����������������
    {
        cout << "WSAStartup failed code <<WSAGetlastError()<<";
        return false;
    }

    return true;
}
/*WSADATA wsadata: ����һ���ṹ�壬�����洢 Windows Sockets �����Ϣ��
WSAStartup(MAKEWORD(2, 2), &wsadata):
MAKEWORD(2, 2)��ָ������� Winsock �汾�ţ�2.2 ��ʾ�汾 2.2���ú������ʼ�� Winsock �⡣
&wsadata: ����һ����������������йص�ǰ Windows �׽���ʵ�ֵ���Ϣ��
WSAStartup ���� 0 ��ʾ�ɹ�������ֵ��ʾʧ�ܡ�
WSAGetlastError()���������һ�� Winsock ����Ĵ����롣
�˺������ڳ�ʼ�� Windows �׽��ֿ⣬���� true ��ʾ��ʼ���ɹ���false ��ʾʧ�ܡ�*/
bool close_socket()
{
    if (0 != WSACleanup())
    {
        cout << "WSACleanup failed code <<WSAGetlastError()<<";
        return false;
    }

    return true;
}
/*WSACleanup(): ���� Winsock �⣬�ͷ���Դ�������ط���ֵ����ʾ����ʧ�ܡ�
WSAGetlastError(): �������һ�δ���Ĵ��롣
�˺������ڹر� Winsock �Ⲣ�ͷ���Դ��*/
SOCKET createServerSocket(int PORT)
{
    //�����յ�socket
    //af:��ַЭ���� ipv4 ipv6
    //type������Э������ ��ʽ�׽��� ���ݱ�
    //protocl��ʹ�þ����ĳ������Э��
    SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == INVALID_SOCKET)
    {
        err("socket");
        return INVALID_SOCKET;
    }//����ʧ��

    //��ip�Ͷ˿�
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;//ipv4
    addr.sin_port = htons(PORT);//�˿�,�����ֽ���ת��Ϊ�����ֽ��򣬴�˴洢תС�˴洢
    addr.sin_addr.s_addr = ADDR_ANY;//ip��ַ,inet_addr("127.0.0.1")

    //��
    if (SOCKET_ERROR == bind(fd, (struct sockaddr*)&addr, sizeof(addr)))
    {
        err("bind");
        closesocket(fd); // ��ʧ��ʱ�ر� socket
        return INVALID_SOCKET;
    }

    //������Ϣ
    if (listen(fd, 10) == SOCKET_ERROR)
    {
        err("listen");
        closesocket(fd); // ����ʧ��ʱ�ر� socket
        return INVALID_SOCKET;
    }
    return fd;
}
/*SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP): ����һ����ʽ�׽��֣�ʹ�� TCP Э�顣

AF_INET: ָ��ʹ�� IPv4 ��ַ��
SOCK_STREAM: ָ���׽�������Ϊ��ʽ�׽��֣���ʾʹ�� TCP Э�顣
IPPROTO_TCP: ָ��ʹ�� TCP Э�顣
INVALID_SOCKET: ���� socket() ��������ʧ��ʱ���ص�ֵ����ʾ�׽��ִ���ʧ�ܡ�

struct sockaddr_in addr: ����һ���ṹ�壬��ʾ IPv4 ��ַ�ĸ�ʽ�������������ֶΣ�

sin_family = AF_INET: ָ����ַ��Ϊ IPv4��
sin_port = htons(PORT): �˿ںţ�ͨ�� htons �����������ֽ���תΪ�����ֽ��򣨴�˴洢����
sin_addr.s_addr = ADDR_ANY: ��ʾ�󶨵����п��õ�����ӿڣ�ADDR_ANY ��ָ�����κε�ַ���ӡ�
bind(fd, (struct sockaddr*)&addr, sizeof(addr)): ���׽��� fd ��ָ���ĵ�ַ�Ͷ˿ڡ�

fd ��֮ǰ�������׽�����������
(struct sockaddr*)&addr �ǽ� sockaddr_in ����ת��Ϊ sockaddr ���͡�
sizeof(addr) �ǵ�ַ�ṹ�Ĵ�С��
listen(fd, 10): ���׽��� fd ����Ϊ����ģʽ��10 ��ʾ������� 10 ���ͻ��������Ŷӵȴ���

closesocket(fd): �ر��׽��֡�

�ú�������һ�� TCP �������׽��֣��󶨵�ָ���˿ڣ�����Ϊ����ģʽ�������׽�����������������ִ��󣬷��� INVALID_SOCKET��*/
SOCKET createClientSocket(const char* ip,int PORT)
{
    //�����յ�socket
    //af:��ַЭ���� ipv4 ipv6
    //type������Э������ ��ʽ�׽��� ���ݱ�
    //protocl��ʹ�þ����ĳ������Э��
    SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == INVALID_SOCKET)
    {
        err("socket");
        return INVALID_SOCKET;
    }//����ʧ��

    //���������������
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;//ipv4
    addr.sin_port = htons(PORT);//�˿�,�����ֽ���ת��Ϊ�����ֽ��򣬴�˴洢תС�˴洢
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0)
    {
        err("inet_pton");
        closesocket(fd); // ���� IP ��ַʧ��ʱ�ر� socket
        return INVALID_SOCKET;
    }//ip��ַ,inet_addr("127.0.0.1")
    if (INVALID_SOCKET == connect(fd, (struct sockaddr*)&addr, sizeof(addr)))
    {
        err("connect");
        closesocket(fd); // ����ʧ��ʱ�ر� socket
        return INVALID_SOCKET;
    }
    /*SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP): ����һ����ʽ�׽��֣�ʹ�� TCP Э�顣���������� createServerSocket ����ͬ��

struct sockaddr_in addr: �����������ͬ�����ڴ洢�������� IP ��ַ�Ͷ˿ںš�

inet_pton(AF_INET, ip, &addr.sin_addr): ���ַ�����ʽ�� IP ��ַ������ "127.0.0.1"��ת��Ϊ�����ֽ���Ķ����Ƹ�ʽ���洢�� addr.sin_addr �С�

AF_INET: ��ʾ IPv4 ��ַ�塣
ip: ������ַ�����ʽ�� IP ��ַ��
&addr.sin_addr: �洢ת����ĵ�ַ��
connect(fd, (struct sockaddr*)&addr, sizeof(addr)): ��ָ���ķ������������ӡ�

fd �ǿͻ����׽��֡�
(struct sockaddr*)&addr �Ƿ������ĵ�ַ��Ϣ��
sizeof(addr) �ǵ�ַ�ṹ�Ĵ�С��
closesocket(fd): �ر��׽��֡�

�ú������ڴ���һ�� TCP �ͻ����׽��֣����ӵ�ָ���ķ������Ͷ˿ڡ��������ʧ�ܣ����� INVALID_SOCKET��*/


    return fd;
}
/*init_socket() ���ڳ�ʼ�� Winsock �⡣
close_socket() �������� Winsock �⡣
createServerSocket(int PORT) ���ڴ���һ�� TCP �������׽��֣��󶨵�ָ���˿ڲ������ͻ������ӡ�
createClientSocket(const char* ip, int PORT) ���ڴ���һ�� TCP �ͻ����׽��֣����ӵ�ָ���ķ����� IP �Ͷ˿ڡ�
����ʹ���� SOCKET ��������ʾ�׽��֣�ͨ�� Winsock �ṩ�� API �������׽��ֵĴ������󶨡����������ӵȲ�����*/
/*SOCKET socket(int af, int type, int protocol);
    af����ַ�壨Address Family��
    AF_INET��IPv4 ��ַ��
    AF_INET6��IPv6 ��ַ��
    AF_UNIX��Unix ���׽��֡�
    type���׽������ͣ�Socket Type��
    SOCK_STREAM����ʽ�׽��֣�ͨ������ TCP ���ӡ�
    SOCK_DGRAM�����ݱ��׽��֣�ͨ������ UDP��
    protocol��Э�����ͣ�ͨ��ѡ�� 0������ϵͳ���Զ�ѡ����ʵ�Э�顣
    IPPROTO_TCP��ʹ�� TCP Э�顣
    IPPROTO_UDP��ʹ�� UDP Э�顣
    ����ֵ��
    �ɹ�������һ���׽�����������SOCKET ���ͣ���
    ʧ�ܣ����� INVALID_SOCKET����Ҫ���� WSAGetLastError() ��ȡ���������Ϣ��
int bind(SOCKET s, const struct sockaddr *addr, int addrlen);
    s�����󶨵��׽��֡�
    addr��ָ�� sockaddr �ṹ��ָ�룬ָ�����ص�ַ�Ͷ˿ڡ�
    addrlen��sockaddr �ṹ�Ĵ�С��ͨ��ʹ�� sizeof(sockaddr_in)��
    ����ֵ��
    �ɹ������� 0��
    ʧ�ܣ����� SOCKET_ERROR����Ҫ���� WSAGetLastError() ��ȡ������롣
int listen(SOCKET s, int backlog);
    s�����������׽��֡�
    backlog��ָ��ϵͳ�ں��ڵȴ����Ӷ����е������������ͨ������Ϊ 5 �� 10��
    �ɹ������� 0��
    ʧ�ܣ����� SOCKET_ERROR����Ҫ���� WSAGetLastError() ��ȡ������롣
SOCKET accept(SOCKET s, struct sockaddr *addr, int *addrlen);
    s�������׽��֡�
    addr��ָ�� sockaddr �ṹ��ָ�룬���ڷ��ؿͻ��˵ĵ�ַ��Ϣ��
    addrlen��ָ��������ָ�룬���ص�ַ�ṹ�Ĵ�С��ͨ���� sizeof(sockaddr_in)����
    �ɹ�������һ���µ��׽�����������������ͻ��˽���ͨ�š�
    ʧ�ܣ����� INVALID_SOCKET����Ҫ���� WSAGetLastError() ��ȡ������롣
int connect(SOCKET s, const struct sockaddr *name, int namelen);
    s�������ӵ��׽��֡�
    name��ָ�� sockaddr �ṹ��ָ�룬������������ IP ��ַ�Ͷ˿ںš�
    namelen��sockaddr �ṹ�Ĵ�С��ͨ��ʹ�� sizeof(sockaddr_in)��
    �ɹ������� 0��
    ʧ�ܣ����� SOCKET_ERROR����Ҫ���� WSAGetLastError() ��ȡ������롣
int send(SOCKET s, const char *buf, int len, int flags);
    s���׽�����������
    buf��ָ��Ҫ���͵����ݻ�������
    len��Ҫ���͵����ݵ��ֽ�����
    flags����־λ��ͨ����Ϊ 0��Ҳ��������Ϊ MSG_DONTROUTE �ȣ���
    �ɹ������ط��͵��ֽ�����
    ʧ�ܣ����� SOCKET_ERROR����Ҫ���� WSAGetLastError() ��ȡ������롣
int recv(SOCKET s, char *buf, int len, int flags);
    s���׽�����������
    buf��ָ��������ݵĻ�������
    len���������Ĵ�С��
    flags����־λ��ͨ����Ϊ 0��Ҳ��������Ϊ MSG_WAITALL �ȣ���
    �ɹ������ؽ��յ����ֽ�����
    ʧ�ܣ����� SOCKET_ERROR����Ҫ���� WSAGetLastError() ��ȡ������롣
    ���ӹرգ����� 0��
*/