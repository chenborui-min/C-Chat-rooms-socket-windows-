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

bool isDayMode = true;  // 白天模式标志
std::mutex drawMutex;   // 用于保护绘图的互斥锁
std::mutex clientMutex; // 用于保护客户端列表的互斥锁
vector<wstring> messages; // 存储接收到的消息
map<SOCKET, string> clients; // 存储客户端列表
/*map 是一个关联容器：
map 是 C++ STL (标准模板库) 中的一个容器，它用于存储键值对（key-value）。
在 map 中，每个元素都有一个唯一的键（key）和一个与之关联的值（value）。
map 是按照键的顺序（默认是按升序）来排列元素的，因此它会自动对键进行排序。
SOCKET 是键（key）：
SOCKET 是 WinSock（Windows 套接字编程）中用于标识套接字的类型，通常是一个整数值。
在这行代码中，SOCKET 被用作 map 的键，表示每个客户端连接的套接字。每个客户端都通过一个独特的 SOCKET 标识。
由于每个客户端在通信过程中会建立一个唯一的套接字，因此 SOCKET 可以用来区分不同的客户端。
string 是值（value）：
每个 SOCKET 键对应一个值，这个值是一个字符串 string，用于存储与客户端相关的信息。
这个 string 可以是客户端的用户名、IP 地址、或任何其它你想要存储的标识信息，目的是方便在程序中追踪和识别每个客户端。*/
std::unordered_set<string> displayedMessages;// 存储已经显示的消息哈希值
void updateClientList();

const string validUsername = "user";
const string validPassword = "password";

bool authenticate(SOCKET clifd) {
    char authbuf[BUFSIZ] = { 0 };
    int recvResult = recv(clifd, authbuf, static_cast<int>(BUFSIZ - 1), 0);
    if (recvResult > 0) {
        authbuf[recvResult] = '\0';
        string authStr(authbuf);
        size_t delimiterPos = authStr.find(':');//size_t 类型的变量，用来存储 find 函数返回的索引位置,size_t 是一个无符号整型，通常用于表示对象的大小或容器的索引，因此用于存储字符串中的位置或长度是合适的。
        if (delimiterPos != string::npos) {
            string username = authStr.substr(0, delimiterPos);//substr 函数接受两个参数：第一个参数是起始位置（0），即从字符串的第一个字符开始提取。第二个参数是子字符串的长度（delimiterPos），即提取长度为 delimiterPos 个字符。
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
    initgraph(800, 600);  // 初始化图形窗口，大小为 800x600
    setbkcolor(WHITE);    // 设置背景颜色为白色
    cleardevice();

    // 绘制按钮
    setfillcolor(LIGHTGRAY);
    fillrectangle(700, 10, 780, 50);
    settextcolor(BLACK);
    outtextxy(710, 20, L"Day/Night");

    // 绘制客户端列表
    updateClientList();

    // 主循环
    while (true) {
        // 检测鼠标点击
        ExMessage msg;
        if (peekmessage(&msg, EX_MOUSE)) { // 使用正确的参数 EX_MOUSE
            if (msg.message == WM_LBUTTONDOWN) {
                if (msg.x >= 700 && msg.x <= 780 && msg.y >= 10 && msg.y <= 50) {
                    // 切换背景颜色
                    isDayMode = !isDayMode;
                    drawMutex.lock();  // 锁住绘图
                    setbkcolor(isDayMode ? WHITE : BLACK);
                    cleardevice();
                    // 重新绘制按钮
                    setfillcolor(LIGHTGRAY);
                    fillrectangle(700, 10, 780, 50);
                    settextcolor(isDayMode ? BLACK : WHITE);
                    outtextxy(710, 20, L"Day/Night");

                    // 重新绘制消息
                    int y = 10;
                    for (const auto& message : ::messages) { // 使用全局作用域解析
                        outtextxy(10, y, message.c_str());
                        y += 20;
                    }

                    // 重新绘制客户端列表
                    updateClientList();

                    drawMutex.unlock();  // 解锁绘图
                }
            }
        }
    }
}

void updateClientList() {
    drawMutex.lock();
    setfillcolor(isDayMode ? WHITE : BLACK);
    solidrectangle(600, 60, 780, 600); // 清除旧的客户端列表
    /*solidrectangle 是一个图形库函数，用来绘制一个实心矩形（即填充颜色的矩形），并将其覆盖在指定的区域上。这个函数在一些图形编程库中用于创建带有背景色的矩形，通常用来绘制、覆盖或填充矩形区域。*/
    settextcolor(isDayMode ? BLACK : WHITE);
    int y = 60;
    int index = 1;
    clientMutex.lock();
    for (const auto& client : clients) {
        wstring clientInfo = L"Client " + to_wstring(index) + L": " + wstring(client.second.begin(), client.second.end());
        //std::pair 是 C++ 标准库中提供的一个模板类，用于存储一对值。每个 std::pair 对象包含两个成员，通常是不同类型的数据。std::pair 可以用来方便地存储和传递一对相关的对象，常见的应用场景包括键值对（例如在 std::map 或 std::unordered_map 中）和在需要同时返回多个值的函数中。
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
        int recvResult = recv(clifd, recvbuf, static_cast<int>(BUFSIZ - 1), 0);  // 确保缓冲区不会越界
        if (recvResult > 0) {
            recvbuf[recvResult] = '\0';  // 确保以空字符结尾
            // 使用 UTF-8 编码转换为宽字符串
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring wstr = converter.from_bytes(recvbuf);
            if (!wstr.empty()) {
                // 获取当前时间戳，用于生成唯一标识符
                auto now = chrono::system_clock::now();
                time_t now_time = chrono::system_clock::to_time_t(now);
                tm local_tm;
                localtime_s(&local_tm, &now_time);
                char timebuf[20];
                strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &local_tm);

                // 生成消息的唯一标识符（内容 + 时间 + 来源）
                string messageKey = converter.to_bytes(wstr) + timebuf + to_string(clifd);

                // 检查该消息是否已经显示过
                if (displayedMessages.find(messageKey) == displayedMessages.end()) {
                    // 如果该消息没有出现过，则处理并显示
                    displayedMessages.insert(messageKey);  // 记录已经显示过的消息

                    // 处理私聊消息
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
                        // 广播消息给所有客户端
                        clientMutex.lock();
                        for (const auto& client : clients) {
                            if (client.first != clifd) { // 不发送给自己
                                std::string msgStr = converter.to_bytes(wstr + L" from client " + to_wstring(clifd));
                                if (send(client.first, msgStr.c_str(), msgStr.length(), 0) == SOCKET_ERROR) {
                                    cerr << "send error: " << WSAGetLastError() << endl;
                                }
                            }
                        }
                        clientMutex.unlock();

                        // 在接收方的图形界面上显示消息
                        drawMutex.lock();  // 锁住绘图
                        std::wstring displayMessage = wstr + L" from client " + to_wstring(clifd);  // 正确的消息来源
                        ::messages.push_back(displayMessage); // 使用全局作用域解析
                        settextcolor(isDayMode ? BLACK : WHITE);
                        outtextxy(10, 10 + 20 * (::messages.size() - 1), displayMessage.c_str());
                        drawMutex.unlock();  // 解锁绘图
                    }
                }
            }
        }
        else if (recvResult == 0) {
            // 连接关闭
            cerr << "Connection closed by client" << endl;
            break;
        }
        else {
            // 错误发生，打印错误信息
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

        // 获取当前时间
        auto now = chrono::system_clock::now();
        time_t now_time = chrono::system_clock::to_time_t(now);
        tm local_tm;
        localtime_s(&local_tm, &now_time);
        char timebuf[20];
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &local_tm);

        // 附加时间戳到消息
        message += " (" + string(timebuf) + ")";

        if (message.find("@all") == 0) {
            // 发送给所有客户端
            clientMutex.lock();
            for (const auto& client : clients) {
                if (send(client.first, sendbuf, strlen(sendbuf), 0) == SOCKET_ERROR) {
                    cerr << "send error: " << WSAGetLastError() << endl;
                }
            }
            clientMutex.unlock();
        }
        else if (message.find("@") == 0) {
            // 发送给特定客户端，私聊消息格式为 @clientID message
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

    // 添加到客户端列表
    clientMutex.lock();
    clients[clifd] = "client" + to_string(clifd); // 这里可以根据实际情况设置客户端标识
    clientMutex.unlock();

    updateClientList(); // 更新客户端列表

    thread recvThread(receiveMessages, clifd);
    thread sendThread(sendMessages, clifd);

    recvThread.join();
    sendThread.join();

    // 从客户端列表中移除
    clientMutex.lock();
    clients.erase(clifd);
    clientMutex.unlock();

    updateClientList(); // 更新客户端列表
}

int main()
{
    if (!init_socket()) {
        cout << "Failed to initialize socket library" << endl;
        return -1;
    }

    SOCKET serfd = createServerSocket(12345); // 指定端口号
    if (serfd == INVALID_SOCKET) {
        cout << "Failed to create server socket" << endl;
        close_socket();
        return -1;
    }

    cout << "Server is running and waiting for client connection..." << endl;

    // 创建绘图线程
    thread drawThread(drawChatWindow);

    while (true) {
		SOCKET clifd = accept(serfd, NULL, NULL);//accept 函数用于接受客户端的连接请求，并返回一个新的套接字描述符，用于与客户端进行通信。accept 函数的第一个参数是服务器套接字描述符，第二个和第三个参数分别是指向 sockaddr 结构体的指针，用于存储客户端的 IP 地址和端口号。
        if (clifd == INVALID_SOCKET) {
            cerr << "accept error: " << WSAGetLastError() << endl;
            closesocket(serfd);
            close_socket();
            return -1;
        }

        cout << "Client connected!" << endl;

		thread clientThread(handleClient, clifd);//创建一个新线程，用于处理客户端连接。handleClient 函数用于处理客户端连接，它接受一个 SOCKET 参数，表示客户端的套接字描述符。
        clientThread.detach();
        /*std::thread 类提供了 detach() 方法，它会将线程与 std::thread 对象分离。
            当你调用 detach() 后，线程将继续在后台执行，直到完成。此时，不再需要或不可能再通过 std::thread 对象等待线程结束（即不能再调用 join()）。
            一旦线程被分离，它将成为一个独立的线程，操作系统会负责管理它的生命周期。如果线程已经完成，它的资源会被回收；如果线程还在运行，它会在后台继续执行。
            为什么要使用 detach()？
            detach() 适用于你希望线程在后台独立运行的情况，且不需要再等待它的结果或知道它什么时候结束。*/
    }

    drawThread.join();
    closesocket(serfd);
    close_socket();

    cout << "-----Server closed.-----" << endl;
    return 0;
}
