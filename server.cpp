#include "../tcpsocket/tcpsocket.h"
#include <iostream>
#include <thread>
#include <easyx.h>
#include <mutex>
#include <vector>
#include <locale>
#include <codecvt>
#include <map>
#include <string>
#include <chrono>
#include <ctime>
#include <unordered_set>
using namespace std;

bool isDayMode = true;  // ����ģʽ��־
std::mutex drawMutex;   // ���ڱ�����ͼ�Ļ�����
std::mutex clientMutex; // ���ڱ����ͻ����б�Ļ�����
vector<wstring> messages; // �洢���յ�����Ϣ
map<SOCKET, string> clients; // �洢�ͻ����б�
/*map ��һ������������
map �� C++ STL (��׼ģ���) �е�һ�������������ڴ洢��ֵ�ԣ�key-value����
�� map �У�ÿ��Ԫ�ض���һ��Ψһ�ļ���key����һ����֮������ֵ��value����
map �ǰ��ռ���˳��Ĭ���ǰ�����������Ԫ�صģ���������Զ��Լ���������
SOCKET �Ǽ���key����
SOCKET �� WinSock��Windows �׽��ֱ�̣������ڱ�ʶ�׽��ֵ����ͣ�ͨ����һ������ֵ��
�����д����У�SOCKET ������ map �ļ�����ʾÿ���ͻ������ӵ��׽��֡�ÿ���ͻ��˶�ͨ��һ�����ص� SOCKET ��ʶ��
����ÿ���ͻ�����ͨ�Ź����лὨ��һ��Ψһ���׽��֣���� SOCKET �����������ֲ�ͬ�Ŀͻ��ˡ�
string ��ֵ��value����
ÿ�� SOCKET ����Ӧһ��ֵ�����ֵ��һ���ַ��� string�����ڴ洢��ͻ�����ص���Ϣ��
��� string �����ǿͻ��˵��û�����IP ��ַ�����κ���������Ҫ�洢�ı�ʶ��Ϣ��Ŀ���Ƿ����ڳ�����׷�ٺ�ʶ��ÿ���ͻ��ˡ�*/
std::unordered_set<string> displayedMessages;// �洢�Ѿ���ʾ����Ϣ��ϣֵ
void updateClientList();

const string validUsername = "user";
const string validPassword = "password";

bool authenticate(SOCKET clifd) {
    char authbuf[BUFSIZ] = { 0 };
    int recvResult = recv(clifd, authbuf, static_cast<int>(BUFSIZ - 1), 0);
    if (recvResult > 0) {
        authbuf[recvResult] = '\0';
        string authStr(authbuf);
        size_t delimiterPos = authStr.find(':');//size_t ���͵ı����������洢 find �������ص�����λ��,size_t ��һ���޷������ͣ�ͨ�����ڱ�ʾ����Ĵ�С��������������������ڴ洢�ַ����е�λ�û򳤶��Ǻ��ʵġ�
        if (delimiterPos != string::npos) {
            string username = authStr.substr(0, delimiterPos);//substr ��������������������һ����������ʼλ�ã�0���������ַ����ĵ�һ���ַ���ʼ��ȡ���ڶ������������ַ����ĳ��ȣ�delimiterPos��������ȡ����Ϊ delimiterPos ���ַ���
            string password = authStr.substr(delimiterPos + 1);
            if (username == validUsername && password == validPassword) {
                send(clifd, "OK", 2, 0);
                return true;
            }
        }
    }
    send(clifd, "FAIL", 4, 0);
    return false;
}

void drawChatWindow() {
    initgraph(800, 600);  // ��ʼ��ͼ�δ��ڣ���СΪ 800x600
    setbkcolor(WHITE);    // ���ñ�����ɫΪ��ɫ
    cleardevice();

    // ���ư�ť
    setfillcolor(LIGHTGRAY);
    fillrectangle(700, 10, 780, 50);
    settextcolor(BLACK);
    outtextxy(710, 20, L"Day/Night");

    // ���ƿͻ����б�
    updateClientList();

    // ��ѭ��
    while (true) {
        // ��������
        ExMessage msg;
        if (peekmessage(&msg, EX_MOUSE)) { // ʹ����ȷ�Ĳ��� EX_MOUSE
            if (msg.message == WM_LBUTTONDOWN) {
                if (msg.x >= 700 && msg.x <= 780 && msg.y >= 10 && msg.y <= 50) {
                    // �л�������ɫ
                    isDayMode = !isDayMode;
                    drawMutex.lock();  // ��ס��ͼ
                    setbkcolor(isDayMode ? WHITE : BLACK);
                    cleardevice();
                    // ���»��ư�ť
                    setfillcolor(LIGHTGRAY);
                    fillrectangle(700, 10, 780, 50);
                    settextcolor(isDayMode ? BLACK : WHITE);
                    outtextxy(710, 20, L"Day/Night");

                    // ���»�����Ϣ
                    int y = 10;
                    for (const auto& message : ::messages) { // ʹ��ȫ�����������
                        outtextxy(10, y, message.c_str());
                        y += 20;
                    }

                    // ���»��ƿͻ����б�
                    updateClientList();

                    drawMutex.unlock();  // ������ͼ
                }
            }
        }
    }
}

void updateClientList() {
    drawMutex.lock();
    setfillcolor(isDayMode ? WHITE : BLACK);
    solidrectangle(600, 60, 780, 600); // ����ɵĿͻ����б�
    /*solidrectangle ��һ��ͼ�ο⺯������������һ��ʵ�ľ��Σ��������ɫ�ľ��Σ��������串����ָ���������ϡ����������һЩͼ�α�̿������ڴ������б���ɫ�ľ��Σ�ͨ���������ơ����ǻ�����������*/
    settextcolor(isDayMode ? BLACK : WHITE);
    int y = 60;
    int index = 1;
    clientMutex.lock();
    for (const auto& client : clients) {
        wstring clientInfo = L"Client " + to_wstring(index) + L": " + wstring(client.second.begin(), client.second.end());
        //std::pair �� C++ ��׼�����ṩ��һ��ģ���࣬���ڴ洢һ��ֵ��ÿ�� std::pair �������������Ա��ͨ���ǲ�ͬ���͵����ݡ�std::pair ������������ش洢�ʹ���һ����صĶ��󣬳�����Ӧ�ó���������ֵ�ԣ������� std::map �� std::unordered_map �У�������Ҫͬʱ���ض��ֵ�ĺ����С�
        outtextxy(600, y, clientInfo.c_str());
        y += 20;
        index++;
    }
    clientMutex.unlock();
    drawMutex.unlock();
}

void receiveMessages(SOCKET clifd) {
    char recvbuf[BUFSIZ] = { 0 };
    while (true) {
        int recvResult = recv(clifd, recvbuf, static_cast<int>(BUFSIZ - 1), 0);  // ȷ������������Խ��
        if (recvResult > 0) {
            recvbuf[recvResult] = '\0';  // ȷ���Կ��ַ���β
            // ʹ�� UTF-8 ����ת��Ϊ���ַ���
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring wstr = converter.from_bytes(recvbuf);
            if (!wstr.empty()) {
                // ��ȡ��ǰʱ�������������Ψһ��ʶ��
                auto now = chrono::system_clock::now();
                time_t now_time = chrono::system_clock::to_time_t(now);
                tm local_tm;
                localtime_s(&local_tm, &now_time);
                char timebuf[20];
                strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &local_tm);

                // ������Ϣ��Ψһ��ʶ�������� + ʱ�� + ��Դ��
                string messageKey = converter.to_bytes(wstr) + timebuf + to_string(clifd);

                // ������Ϣ�Ƿ��Ѿ���ʾ��
                if (displayedMessages.find(messageKey) == displayedMessages.end()) {
                    // �������Ϣû�г��ֹ���������ʾ
                    displayedMessages.insert(messageKey);  // ��¼�Ѿ���ʾ������Ϣ

                    // ����˽����Ϣ
                    if (wstr.find(L"@") == 0) {
                        size_t pos = wstr.find(L' ');
                        if (pos != std::wstring::npos) {
                            std::wstring targetClient = wstr.substr(1, pos - 1);
                            std::wstring msg = wstr.substr(pos + 1);

                            clientMutex.lock();
                            for (const auto& client : clients) {
                                if (client.second == std::string(targetClient.begin(), targetClient.end())) {
                                    std::string msgStr = converter.to_bytes(msg + L" from client " + to_wstring(clifd));
                                    if (send(client.first, msgStr.c_str(), msgStr.length(), 0) == SOCKET_ERROR) {
                                        cerr << "send error: " << WSAGetLastError() << endl;
                                    }
                                    break;
                                }
                            }
                            clientMutex.unlock();
                        }
                    }
                    else {
                        // �㲥��Ϣ�����пͻ���
                        clientMutex.lock();
                        for (const auto& client : clients) {
                            if (client.first != clifd) { // �����͸��Լ�
                                std::string msgStr = converter.to_bytes(wstr + L" from client " + to_wstring(clifd));
                                if (send(client.first, msgStr.c_str(), msgStr.length(), 0) == SOCKET_ERROR) {
                                    cerr << "send error: " << WSAGetLastError() << endl;
                                }
                            }
                        }
                        clientMutex.unlock();

                        // �ڽ��շ���ͼ�ν�������ʾ��Ϣ
                        drawMutex.lock();  // ��ס��ͼ
                        std::wstring displayMessage = wstr + L" from client " + to_wstring(clifd);  // ��ȷ����Ϣ��Դ
                        ::messages.push_back(displayMessage); // ʹ��ȫ�����������
                        settextcolor(isDayMode ? BLACK : WHITE);
                        outtextxy(10, 10 + 20 * (::messages.size() - 1), displayMessage.c_str());
                        drawMutex.unlock();  // ������ͼ
                    }
                }
            }
        }
        else if (recvResult == 0) {
            // ���ӹر�
            cerr << "Connection closed by client" << endl;
            break;
        }
        else {
            // ����������ӡ������Ϣ
            cerr << "recv failed: " << WSAGetLastError() << endl;
            break;
        }
    }
}

void sendMessages(SOCKET clifd) {
    char sendbuf[BUFSIZ] = { 0 };
    while (true) {
        cin.getline(sendbuf, BUFSIZ);
        string message(sendbuf);

        // ��ȡ��ǰʱ��
        auto now = chrono::system_clock::now();
        time_t now_time = chrono::system_clock::to_time_t(now);
        tm local_tm;
        localtime_s(&local_tm, &now_time);
        char timebuf[20];
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &local_tm);

        // ����ʱ�������Ϣ
        message += " (" + string(timebuf) + ")";

        if (message.find("@all") == 0) {
            // ���͸����пͻ���
            clientMutex.lock();
            for (const auto& client : clients) {
                if (send(client.first, sendbuf, strlen(sendbuf), 0) == SOCKET_ERROR) {
                    cerr << "send error: " << WSAGetLastError() << endl;
                }
            }
            clientMutex.unlock();
        }
        else if (message.find("@") == 0) {
            // ���͸��ض��ͻ��ˣ�˽����Ϣ��ʽΪ @clientID message
            size_t pos = message.find(' ');
            if (pos != string::npos) {
                string targetClient = message.substr(1, pos - 1);
                string msg = message.substr(pos + 1);

                clientMutex.lock();
                for (const auto& client : clients) {
                    if (client.second == targetClient) {
                        if (send(client.first, msg.c_str(), msg.length(), 0) == SOCKET_ERROR) {
                            cerr << "send error: " << WSAGetLastError() << endl;
                        }
                        break;
                    }
                }
                clientMutex.unlock();
            }
        }

        memset(sendbuf, 0, BUFSIZ);
    }
    closesocket(clifd);
}

void handleClient(SOCKET clifd) {
    if (!authenticate(clifd)) {
        cerr << "Authentication failed for client: " << clifd << endl;
        closesocket(clifd);
        return;
    }
    cout << "Client authenticated: " << clifd << endl;

    // ��ӵ��ͻ����б�
    clientMutex.lock();
    clients[clifd] = "client" + to_string(clifd); // ������Ը���ʵ��������ÿͻ��˱�ʶ
    clientMutex.unlock();

    updateClientList(); // ���¿ͻ����б�

    thread recvThread(receiveMessages, clifd);
    thread sendThread(sendMessages, clifd);

    recvThread.join();
    sendThread.join();

    // �ӿͻ����б����Ƴ�
    clientMutex.lock();
    clients.erase(clifd);
    clientMutex.unlock();

    updateClientList(); // ���¿ͻ����б�
}

int main()
{
    if (!init_socket()) {
        cout << "Failed to initialize socket library" << endl;
        return -1;
    }

    SOCKET serfd = createServerSocket(12345); // ָ���˿ں�
    if (serfd == INVALID_SOCKET) {
        cout << "Failed to create server socket" << endl;
        close_socket();
        return -1;
    }

    cout << "Server is running and waiting for client connection..." << endl;

    // ������ͼ�߳�
    thread drawThread(drawChatWindow);

    while (true) {
		SOCKET clifd = accept(serfd, NULL, NULL);//accept �������ڽ��ܿͻ��˵��������󣬲�����һ���µ��׽�����������������ͻ��˽���ͨ�š�accept �����ĵ�һ�������Ƿ������׽������������ڶ����͵����������ֱ���ָ�� sockaddr �ṹ���ָ�룬���ڴ洢�ͻ��˵� IP ��ַ�Ͷ˿ںš�
        if (clifd == INVALID_SOCKET) {
            cerr << "accept error: " << WSAGetLastError() << endl;
            closesocket(serfd);
            close_socket();
            return -1;
        }

        cout << "Client connected!" << endl;

		thread clientThread(handleClient, clifd);//����һ�����̣߳����ڴ���ͻ������ӡ�handleClient �������ڴ���ͻ������ӣ�������һ�� SOCKET ��������ʾ�ͻ��˵��׽�����������
        clientThread.detach();
        /*std::thread ���ṩ�� detach() ���������Ὣ�߳��� std::thread ������롣
            ������� detach() ���߳̽������ں�ִ̨�У�ֱ����ɡ���ʱ��������Ҫ�򲻿�����ͨ�� std::thread ����ȴ��߳̽������������ٵ��� join()����
            һ���̱߳����룬������Ϊһ���������̣߳�����ϵͳ�Ḻ����������������ڡ�����߳��Ѿ���ɣ�������Դ�ᱻ���գ�����̻߳������У������ں�̨����ִ�С�
            ΪʲôҪʹ�� detach()��
            detach() ��������ϣ���߳��ں�̨�������е�������Ҳ���Ҫ�ٵȴ����Ľ����֪����ʲôʱ�������*/
    }

    drawThread.join();
    closesocket(serfd);
    close_socket();

    cout << "-----Server closed.-----" << endl;
    return 0;
}
