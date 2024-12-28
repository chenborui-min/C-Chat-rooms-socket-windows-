#include "../tcpsocket/tcpsocket.h"
#include <iostream>
#include <thread>
#include <easyx.h>
#include <mutex>
#include <vector>
#include <locale>
#include <codecvt>
#include <chrono>
#include <ctime>
#include <string>
#include <unordered_set>  // 用于存储消息标识符
/*tcpsocket.h: 提供初始化、创建、关闭套接字的函数。
easyx.h: 用于绘制图形界面。
thread: 实现消息收发与图形界面分离。
mutex: 保护共享资源（绘图）。
vector 和 unordered_set: 分别用于存储消息和避免重复显示*/
using namespace std;

bool isDayMode = true;  // 模式标志
std::mutex drawMutex;   // 用于保护绘图的互斥锁，用于在多线程编程中确保同一时间只有一个线程可以访问某个共享资源。它的目的是防止多个线程同时访问共享资源，避免数据竞争和不一致的情况。
vector<wstring> messages; // 存储接收到的消息，是 C++ 标准库中的宽字符字符串类型，通常用于存储包含 Unicode 字符的字符串。wstring 中的字符是 wchar_t 类型的，每个字符占据 2 或 4 个字节（取决于平台和编译器）。wstring 比 std::string 更适用于处理国际化字符集，例如中文、日文等非 ASCII 字符。
// 存储已经显示过的消息标识符用于存储唯一的字符串元素，底层实现使用哈希表，因此能提供快速的访问效率。它的特点是元素的存储顺序不固定，即元素是无序的。
//唯一性：每个元素在集合中只能出现一次。如果尝试插入重复的元素，插入会失败。
//无序：元素的存储顺序不确定，遍历时的顺序与插入顺序无关。
//高效性：平均情况下，插入、删除和查找操作的时间复杂度为 O(1)，但在最坏的情况下，当哈希冲突严重时，可能会退化为 O(n)。
std::unordered_set<string> displayedMessages;
/*1. 底层实现：
set：使用 红黑树（一种自平衡的二叉搜索树）作为底层数据结构，因此元素会保持有序状态。它根据元素的比较操作（通常是通过 < 操作符）来维护元素的顺序。
unordered_set：使用 哈希表 作为底层数据结构，元素的顺序是无序的，查找、插入和删除操作通过哈希函数来决定元素的位置。
2. 元素顺序：
set：元素是有序的，按照从小到大的顺序排列（或根据自定义的比较函数进行排序）。
unordered_set：元素是无序的，遍历元素时的顺序不固定。
3. 操作复杂度：
set：
插入、删除和查找的平均时间复杂度为 O(log n)，因为底层是红黑树，操作是通过树的结构进行的。
unordered_set：
插入、删除和查找的平均时间复杂度为 O(1)，因为它使用哈希表（哈希冲突可能导致复杂度退化，但一般情况下可以保持常数时间）。
4. 内存使用：
set：由于使用红黑树，它的内存开销相对较小，但需要额外存储树的结构（节点链接等）。
unordered_set：由于使用哈希表，可能需要较大的内存来存储哈希桶和避免冲突。
5. 适用场景：
set：当你需要保证元素的有序性，并且对元素的顺序有要求时，使用 set。例如，如果需要从小到大遍历元素，或者进行区间查找等操作，set 是一个合适的选择。
unordered_set：当你关心的是快速查找、插入和删除，而不需要关心元素的顺序时，使用 unordered_set。它在查找、插入和删除时通常表现更好，尤其是在元素数目较大时。*/
void drawChatWindow() {
    initgraph(800, 600);  // 初始化图形窗口，大小为 800x600
    setbkcolor(WHITE);    // 设置背景颜色为白色
	cleardevice();		// 清空窗口

    // 绘制按钮
    setfillcolor(LIGHTGRAY);
    fillrectangle(700, 10, 780, 50);
    settextcolor(BLACK);
	outtextxy(710, 20, L"Day/Night");//L表示宽字符字符串

    // 主循环
    while (true) {
        // 检测鼠标点击
        ExMessage msg;/*struct ExMessage {
    int message;  // 消息类型，WM_LBUTTONDOWN
    int x, y;     // 鼠标的 x 和 y 坐标
    int wParam;   // 鼠标按钮状态，通常在鼠标消息中用于附加信息
    int lParam;   // 鼠标位置的附加信息，存储 x 和 y 坐标
    int type;     // 事件类型，如鼠标点击、鼠标移动等
};*/
        if (peekmessage(&msg, EX_MOUSE)) { // 用于从消息队列中检索消息，但 不会移除 消息。它允许你在不从消息队列中移除消息的情况下查看消息内容，因此，它常用于检查是否有事件发生，或者处理用户输入而不打断程序的主要逻辑。EX_MOUSE 是 peekmessage 的参数之一，表示我们只关心鼠标相关的消息。
			if (msg.message == WM_LBUTTONDOWN) {// 表示鼠标左键按下,WM_RBUTTONDOWN 表示鼠标右键按下
				if (msg.x >= 700 && msg.x <= 780 && msg.y >= 10 && msg.y <= 50) {// 检查鼠标点击的位置是否在按钮范围内
                    // 切换背景颜色
                    isDayMode = !isDayMode;//通过取反操作 ! 来切换其状态。
                    drawMutex.lock();  // 锁住绘图
					setbkcolor(isDayMode ? WHITE : BLACK);  // 根据当前模式设置背景颜色
                    cleardevice();
                    // 重新绘制按钮
                    setfillcolor(LIGHTGRAY);
                    fillrectangle(700, 10, 780, 50);
                    settextcolor(isDayMode ? BLACK : WHITE);
                    outtextxy(710, 20, L"Day/Night");

                    // 重新绘制消息
                    int y = 10;
                    for (const auto& message : ::messages) { // 访问全局变量 messages，const auto& 是 C++11 引入的类型推导特性，用于自动推导元素类型并以引用的方式访问元素，同时保证元素不被修改。
                        /*std::vector<std::wstring> messages = {L"Hello", L"World", L"Chat Message"};
                            for (const auto& message : messages) {
                                    std::wcout << message << std::endl;  // 输出每个消息
                                }*/
                        outtextxy(10, y, message.c_str());//message.c_str() 将 message 转换为 C 风格的字符串，以便绘制。.c_str()：这是 std::string 和 std::wstring 类的成员函数，用于返回一个常量字符指针（const char* 或 const wchar_t*），指向字符串对象的数据存储区的起始位置。
                        /*与 C 风格 API 兼容：许多 C 风格的库（比如一些 Windows API、标准 C 函数等）都要求传入 C 风格字符串（const char* 或 const wchar_t*）。
                                因此，std::string 和 std::wstring 提供了 c_str() 方法来方便地将其转换为 C 风格字符串。
                        输出字符串：可以使用 message.c_str() 将 std::wstring 或 std::string 转换为 C 风格字符串，然后传递给需要 C 风格字符串的函数（如 printf 或 outtextxy 等）。*/
                        y += 20;
                    }
                    drawMutex.unlock();  // 解锁绘图
                }
            }
        }
    }
}

void receiveMessages(SOCKET clifd) {
    char recvbuf[BUFSIZ] = { 0 };
    int y = 10;  // 用于记录文本的Y坐标
    while (true) {
        int recvResult = recv(clifd, recvbuf, static_cast<int>(BUFSIZ - 1), 0); 
        if (recvResult > 0) {
            recvbuf[recvResult] = '\0';  // 确保以空字符结尾
            // 使用 UTF-8 编码转换为宽字符串
            /*UTF-8 是一种变长字符编码，它使用 1 至 4 字节来表示一个字符，广泛用于网络传输和文件存储。
                UTF-16 是一种变长字符编码，它使用 2 或 4 字节来表示一个字符，广泛用于 Windows 平台上的字符处理。*/
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;//wstring_convert 对象 converter，用于将 UTF-8 编码的字符串转换为 UTF-16 编码的宽字符串，std::codecvt_utf8_utf16<wchar_t> 是一个转换工具，用于在 UTF-8 和 UTF-16 之间进行转换。
            std::wstring wstr = converter.from_bytes(recvbuf);//from_bytes 是 std::wstring_convert 类的成员函数，用于将 UTF-8 编码的字节数据（std::string 或 char[]）转换为 UTF-16 编码的宽字符字符串（std::wstring）。
            /*std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter; 是一个用于将 UTF-8 编码的字节流转换为 UTF-16 编码（宽字符）的 std::wstring 字符串的转换器。
                from_bytes 函数通过将 UTF-8 编码的字节数据逐个字符解码，并返回对应的宽字符字符串（std::wstring）。*/
            if (!wstr.empty()) {
				// 确定消息来源是来自客户端还是服务器//????????????????????
                std::wstring sender = (clifd == INVALID_SOCKET) ? L"from server" : L"from client " + std::to_wstring(clifd);
                wstr += L" " + sender; // 将来源添加到消息后面

                // 获取当前时间戳，用于生成唯一标识符
                auto now = chrono::system_clock::now();
                time_t now_time = chrono::system_clock::to_time_t(now);
                tm local_tm;
                localtime_s(&local_tm, &now_time);
                char timebuf[20];
                strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &local_tm);

                // 生成消息的唯一标识符（内容 + 时间 + 来源）,将宽字符字符串（std::wstring）wstr 转换为 UTF-8 编码的字节流（std::string）。
                std::string messageKey = converter.to_bytes(wstr) + timebuf + std::to_string(clifd);

                // 检查该消息是否已经显示过
                if (displayedMessages.find(messageKey) == displayedMessages.end()) {
                    // 如果该消息没有出现过，则处理并显示
                    displayedMessages.insert(messageKey);  // 记录已经显示过的消息

                    drawMutex.lock();  // 锁住绘图
                    ::messages.push_back(wstr); // 使用全局作用域解析,用于将一个元素添加到 vector 的末尾。
                    settextcolor(isDayMode ? BLACK : WHITE);
                    outtextxy(10, y, wstr.c_str());
                    y += 20;  // 每次接收到消息后，Y坐标增加20，以便显示在下一行
                    drawMutex.unlock();  // 解锁绘图
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
        auto now = chrono::system_clock::now();//将由编译器自动推导为 chrono::system_clock::time_point，这是 system_clock::now() 返回的类型。
        time_t now_time = chrono::system_clock::to_time_t(now);//chrono::system_clock::to_time_t(now) 将 chrono::system_clock::time_point 类型的 now 转换为 time_t 类型的 now_time，使得 now_time 可以与 C 库的时间相关函数（如 localtime 或 gmtime）兼容。
        //time_t 是 C 和 C++ 中用于表示时间的传统类型。它通常是一个整型，用来表示自1970年1月1日（称为“UNIX纪元”）以来的秒数。
        tm local_tm;//tm 类型是一个结构体，表示一个具体的时间点，通常用于存储由 time_t 类型
        /*struct tm {
                int tm_sec;   // 秒 (0 - 59)
                int tm_min;   // 分钟 (0 - 59)
                int tm_hour;  // 小时 (0 - 23)
                int tm_mday;  // 日 (1 - 31)
                int tm_mon;   // 月 (0 - 11，0 代表 1 月)
                int tm_year;  // 年 (从1900年开始，1900年表示 0)
                int tm_wday;  // 星期几 (0 - 6，0 代表星期天)
                int tm_yday;  // 一年中的第几天 (0 - 365)
                int tm_isdst; // 夏令时标志 (正值表示夏令时，0 表示不使用夏令时，负值表示无法确定)
        };*/
        localtime_s(&local_tm, &now_time);//localtime_s 是线程安全版本，能够将 time_t 时间戳转换为结构体格式（年、月、日、时、分、秒等）。
        char timebuf[20];
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &local_tm);
		//strftime 函数用于将时间格式化为字符串，第一个参数是目标字符串缓冲区，第二个参数是缓冲区大小，第三个参数是格式化字符串，第四个参数是时间结构体。

        // 附加时间戳到消息
        message += " (" + string(timebuf) + ")";

        if (message.find("@") == 0) {//返回子字符串首次出现的位置（从 0 开始的索引）。如果未找到指定的子字符串，则返回 std::string::npos。
            // 私聊消息格式为 @clientID message
            if (send(clifd, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
                cerr << "send error: " << WSAGetLastError() << endl;
                break;
            }
        }
        else {
            // 群聊消息，不用加@
            if (send(clifd, ("@all " + message).c_str(), message.length() + 5, 0) == SOCKET_ERROR) {
                cerr << "send error: " << WSAGetLastError() << endl;
                break;
            }
        }

        if (send(clifd, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
            cerr << "send error: " << WSAGetLastError() << endl;
            break;
        }
		memset(sendbuf, 0, BUFSIZ);//memset 函数用于将指定的内存区域设置为指定的值，通常用于初始化内存区域。
    }
    closesocket(clifd);
}

bool authenticate(SOCKET clifd) {
    string username, password;
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;

    string authStr = username + ":" + password;//方便地将其发送到服务器进行验证。
    if (send(clifd, authStr.c_str(), authStr.length(), 0) == SOCKET_ERROR) {
        cerr << "send error: " << WSAGetLastError() << endl;
        return false;
    }

    char recvbuf[BUFSIZ] = { 0 };
    int recvResult = recv(clifd, recvbuf, static_cast<int>(BUFSIZ - 1), 0);//类型转换 操作符，用于将一个表达式的值转换为目标类型。
    if (recvResult > 0) {
        recvbuf[recvResult] = '\0';
        string response(recvbuf);//将 recvbuf 转换为 string 类型，便于后续的比较操作。response 存储了服务器的响应消息。
        return response == "OK";
    }
    return false;
}

int main()
{
    if (!init_socket()) {
        cout << "Failed to initialize socket library" << endl; 
        return -1;
    }

    SOCKET clifd = createClientSocket("127.0.0.1", 12345); // 指定服务器IP和端口号
    if (clifd == INVALID_SOCKET) {
        cout << "Failed to create client socket" << endl;
        close_socket();
        return -1;
    }

    cout << "Connected to server!" << endl;

    if (!authenticate(clifd)) {
        cerr << "Authentication failed" << endl;
		closesocket(clifd);//关闭套接字
		close_socket();//关闭网络库
        return -1;
    }

    // 创建绘图线程
    thread drawThread(drawChatWindow);

    thread recvThread(receiveMessages, clifd);
	thread sendThread(sendMessages, clifd);//创建消息接收和发送线程，分别用于接收和发送消息。

    recvThread.join();
    sendThread.join();
    drawThread.join();//确保 主线程等待 启动的 子线程 执行完成之后，才能继续执行主线程中的后续代码。

    closesocket(clifd);
    close_socket();

    cout << "-----Client closed.-----" << endl;
    return 0;
}

