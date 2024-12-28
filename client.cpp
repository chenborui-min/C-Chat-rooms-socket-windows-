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
#include <unordered_set>  // ���ڴ洢��Ϣ��ʶ��
/*tcpsocket.h: �ṩ��ʼ�����������ر��׽��ֵĺ�����
easyx.h: ���ڻ���ͼ�ν��档
thread: ʵ����Ϣ�շ���ͼ�ν�����롣
mutex: ����������Դ����ͼ����
vector �� unordered_set: �ֱ����ڴ洢��Ϣ�ͱ����ظ���ʾ*/
using namespace std;

bool isDayMode = true;  // ģʽ��־
std::mutex drawMutex;   // ���ڱ�����ͼ�Ļ������������ڶ��̱߳����ȷ��ͬһʱ��ֻ��һ���߳̿��Է���ĳ��������Դ������Ŀ���Ƿ�ֹ����߳�ͬʱ���ʹ�����Դ���������ݾ����Ͳ�һ�µ������
vector<wstring> messages; // �洢���յ�����Ϣ���� C++ ��׼���еĿ��ַ��ַ������ͣ�ͨ�����ڴ洢���� Unicode �ַ����ַ�����wstring �е��ַ��� wchar_t ���͵ģ�ÿ���ַ�ռ�� 2 �� 4 ���ֽڣ�ȡ����ƽ̨�ͱ���������wstring �� std::string �������ڴ�����ʻ��ַ������������ġ����ĵȷ� ASCII �ַ���
// �洢�Ѿ���ʾ������Ϣ��ʶ�����ڴ洢Ψһ���ַ���Ԫ�أ��ײ�ʵ��ʹ�ù�ϣ��������ṩ���ٵķ���Ч�ʡ������ص���Ԫ�صĴ洢˳�򲻹̶�����Ԫ��������ġ�
//Ψһ�ԣ�ÿ��Ԫ���ڼ�����ֻ�ܳ���һ�Ρ�������Բ����ظ���Ԫ�أ������ʧ�ܡ�
//����Ԫ�صĴ洢˳��ȷ��������ʱ��˳�������˳���޹ء�
//��Ч�ԣ�ƽ������£����롢ɾ���Ͳ��Ҳ�����ʱ�临�Ӷ�Ϊ O(1)�������������£�����ϣ��ͻ����ʱ�����ܻ��˻�Ϊ O(n)��
std::unordered_set<string> displayedMessages;
/*1. �ײ�ʵ�֣�
set��ʹ�� �������һ����ƽ��Ķ�������������Ϊ�ײ����ݽṹ�����Ԫ�ػᱣ������״̬��������Ԫ�صıȽϲ�����ͨ����ͨ�� < ����������ά��Ԫ�ص�˳��
unordered_set��ʹ�� ��ϣ�� ��Ϊ�ײ����ݽṹ��Ԫ�ص�˳��������ģ����ҡ������ɾ������ͨ����ϣ����������Ԫ�ص�λ�á�
2. Ԫ��˳��
set��Ԫ��������ģ����մ�С�����˳�����У�������Զ���ıȽϺ����������򣩡�
unordered_set��Ԫ��������ģ�����Ԫ��ʱ��˳�򲻹̶���
3. �������Ӷȣ�
set��
���롢ɾ���Ͳ��ҵ�ƽ��ʱ�临�Ӷ�Ϊ O(log n)����Ϊ�ײ��Ǻ������������ͨ�����Ľṹ���еġ�
unordered_set��
���롢ɾ���Ͳ��ҵ�ƽ��ʱ�临�Ӷ�Ϊ O(1)����Ϊ��ʹ�ù�ϣ����ϣ��ͻ���ܵ��¸��Ӷ��˻�����һ������¿��Ա��ֳ���ʱ�䣩��
4. �ڴ�ʹ�ã�
set������ʹ�ú�����������ڴ濪����Խ�С������Ҫ����洢���Ľṹ���ڵ����ӵȣ���
unordered_set������ʹ�ù�ϣ��������Ҫ�ϴ���ڴ����洢��ϣͰ�ͱ����ͻ��
5. ���ó�����
set��������Ҫ��֤Ԫ�ص������ԣ����Ҷ�Ԫ�ص�˳����Ҫ��ʱ��ʹ�� set�����磬�����Ҫ��С�������Ԫ�أ����߽���������ҵȲ�����set ��һ�����ʵ�ѡ��
unordered_set��������ĵ��ǿ��ٲ��ҡ������ɾ����������Ҫ����Ԫ�ص�˳��ʱ��ʹ�� unordered_set�����ڲ��ҡ������ɾ��ʱͨ�����ָ��ã���������Ԫ����Ŀ�ϴ�ʱ��*/
void drawChatWindow() {
    initgraph(800, 600);  // ��ʼ��ͼ�δ��ڣ���СΪ 800x600
    setbkcolor(WHITE);    // ���ñ�����ɫΪ��ɫ
	cleardevice();		// ��մ���

    // ���ư�ť
    setfillcolor(LIGHTGRAY);
    fillrectangle(700, 10, 780, 50);
    settextcolor(BLACK);
	outtextxy(710, 20, L"Day/Night");//L��ʾ���ַ��ַ���

    // ��ѭ��
    while (true) {
        // ��������
        ExMessage msg;/*struct ExMessage {
    int message;  // ��Ϣ���ͣ�WM_LBUTTONDOWN
    int x, y;     // ���� x �� y ����
    int wParam;   // ��갴ť״̬��ͨ���������Ϣ�����ڸ�����Ϣ
    int lParam;   // ���λ�õĸ�����Ϣ���洢 x �� y ����
    int type;     // �¼����ͣ��������������ƶ���
};*/
        if (peekmessage(&msg, EX_MOUSE)) { // ���ڴ���Ϣ�����м�����Ϣ���� �����Ƴ� ��Ϣ�����������ڲ�����Ϣ�������Ƴ���Ϣ������²鿴��Ϣ���ݣ���ˣ��������ڼ���Ƿ����¼����������ߴ����û����������ϳ������Ҫ�߼���EX_MOUSE �� peekmessage �Ĳ���֮һ����ʾ����ֻ���������ص���Ϣ��
			if (msg.message == WM_LBUTTONDOWN) {// ��ʾ����������,WM_RBUTTONDOWN ��ʾ����Ҽ�����
				if (msg.x >= 700 && msg.x <= 780 && msg.y >= 10 && msg.y <= 50) {// ����������λ���Ƿ��ڰ�ť��Χ��
                    // �л�������ɫ
                    isDayMode = !isDayMode;//ͨ��ȡ������ ! ���л���״̬��
                    drawMutex.lock();  // ��ס��ͼ
					setbkcolor(isDayMode ? WHITE : BLACK);  // ���ݵ�ǰģʽ���ñ�����ɫ
                    cleardevice();
                    // ���»��ư�ť
                    setfillcolor(LIGHTGRAY);
                    fillrectangle(700, 10, 780, 50);
                    settextcolor(isDayMode ? BLACK : WHITE);
                    outtextxy(710, 20, L"Day/Night");

                    // ���»�����Ϣ
                    int y = 10;
                    for (const auto& message : ::messages) { // ����ȫ�ֱ��� messages��const auto& �� C++11 ����������Ƶ����ԣ������Զ��Ƶ�Ԫ�����Ͳ������õķ�ʽ����Ԫ�أ�ͬʱ��֤Ԫ�ز����޸ġ�
                        /*std::vector<std::wstring> messages = {L"Hello", L"World", L"Chat Message"};
                            for (const auto& message : messages) {
                                    std::wcout << message << std::endl;  // ���ÿ����Ϣ
                                }*/
                        outtextxy(10, y, message.c_str());//message.c_str() �� message ת��Ϊ C �����ַ������Ա���ơ�.c_str()������ std::string �� std::wstring ��ĳ�Ա���������ڷ���һ�������ַ�ָ�루const char* �� const wchar_t*����ָ���ַ�����������ݴ洢������ʼλ�á�
                        /*�� C ��� API ���ݣ���� C ���Ŀ⣨����һЩ Windows API����׼ C �����ȣ���Ҫ���� C ����ַ�����const char* �� const wchar_t*����
                                ��ˣ�std::string �� std::wstring �ṩ�� c_str() ����������ؽ���ת��Ϊ C ����ַ�����
                        ����ַ���������ʹ�� message.c_str() �� std::wstring �� std::string ת��Ϊ C ����ַ�����Ȼ�󴫵ݸ���Ҫ C ����ַ����ĺ������� printf �� outtextxy �ȣ���*/
                        y += 20;
                    }
                    drawMutex.unlock();  // ������ͼ
                }
            }
        }
    }
}

void receiveMessages(SOCKET clifd) {
    char recvbuf[BUFSIZ] = { 0 };
    int y = 10;  // ���ڼ�¼�ı���Y����
    while (true) {
        int recvResult = recv(clifd, recvbuf, static_cast<int>(BUFSIZ - 1), 0); 
        if (recvResult > 0) {
            recvbuf[recvResult] = '\0';  // ȷ���Կ��ַ���β
            // ʹ�� UTF-8 ����ת��Ϊ���ַ���
            /*UTF-8 ��һ�ֱ䳤�ַ����룬��ʹ�� 1 �� 4 �ֽ�����ʾһ���ַ����㷺�������紫����ļ��洢��
                UTF-16 ��һ�ֱ䳤�ַ����룬��ʹ�� 2 �� 4 �ֽ�����ʾһ���ַ����㷺���� Windows ƽ̨�ϵ��ַ�����*/
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;//wstring_convert ���� converter�����ڽ� UTF-8 ������ַ���ת��Ϊ UTF-16 ����Ŀ��ַ�����std::codecvt_utf8_utf16<wchar_t> ��һ��ת�����ߣ������� UTF-8 �� UTF-16 ֮�����ת����
            std::wstring wstr = converter.from_bytes(recvbuf);//from_bytes �� std::wstring_convert ��ĳ�Ա���������ڽ� UTF-8 ������ֽ����ݣ�std::string �� char[]��ת��Ϊ UTF-16 ����Ŀ��ַ��ַ�����std::wstring����
            /*std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter; ��һ�����ڽ� UTF-8 ������ֽ���ת��Ϊ UTF-16 ���루���ַ����� std::wstring �ַ�����ת������
                from_bytes ����ͨ���� UTF-8 ������ֽ���������ַ����룬�����ض�Ӧ�Ŀ��ַ��ַ�����std::wstring����*/
            if (!wstr.empty()) {
				// ȷ����Ϣ��Դ�����Կͻ��˻��Ƿ�����//????????????????????
                std::wstring sender = (clifd == INVALID_SOCKET) ? L"from server" : L"from client " + std::to_wstring(clifd);
                wstr += L" " + sender; // ����Դ��ӵ���Ϣ����

                // ��ȡ��ǰʱ�������������Ψһ��ʶ��
                auto now = chrono::system_clock::now();
                time_t now_time = chrono::system_clock::to_time_t(now);
                tm local_tm;
                localtime_s(&local_tm, &now_time);
                char timebuf[20];
                strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &local_tm);

                // ������Ϣ��Ψһ��ʶ�������� + ʱ�� + ��Դ��,�����ַ��ַ�����std::wstring��wstr ת��Ϊ UTF-8 ������ֽ�����std::string����
                std::string messageKey = converter.to_bytes(wstr) + timebuf + std::to_string(clifd);

                // ������Ϣ�Ƿ��Ѿ���ʾ��
                if (displayedMessages.find(messageKey) == displayedMessages.end()) {
                    // �������Ϣû�г��ֹ���������ʾ
                    displayedMessages.insert(messageKey);  // ��¼�Ѿ���ʾ������Ϣ

                    drawMutex.lock();  // ��ס��ͼ
                    ::messages.push_back(wstr); // ʹ��ȫ�����������,���ڽ�һ��Ԫ����ӵ� vector ��ĩβ��
                    settextcolor(isDayMode ? BLACK : WHITE);
                    outtextxy(10, y, wstr.c_str());
                    y += 20;  // ÿ�ν��յ���Ϣ��Y��������20���Ա���ʾ����һ��
                    drawMutex.unlock();  // ������ͼ
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
        auto now = chrono::system_clock::now();//���ɱ������Զ��Ƶ�Ϊ chrono::system_clock::time_point������ system_clock::now() ���ص����͡�
        time_t now_time = chrono::system_clock::to_time_t(now);//chrono::system_clock::to_time_t(now) �� chrono::system_clock::time_point ���͵� now ת��Ϊ time_t ���͵� now_time��ʹ�� now_time ������ C ���ʱ����غ������� localtime �� gmtime�����ݡ�
        //time_t �� C �� C++ �����ڱ�ʾʱ��Ĵ�ͳ���͡���ͨ����һ�����ͣ�������ʾ��1970��1��1�գ���Ϊ��UNIX��Ԫ����������������
        tm local_tm;//tm ������һ���ṹ�壬��ʾһ�������ʱ��㣬ͨ�����ڴ洢�� time_t ����
        /*struct tm {
                int tm_sec;   // �� (0 - 59)
                int tm_min;   // ���� (0 - 59)
                int tm_hour;  // Сʱ (0 - 23)
                int tm_mday;  // �� (1 - 31)
                int tm_mon;   // �� (0 - 11��0 ���� 1 ��)
                int tm_year;  // �� (��1900�꿪ʼ��1900���ʾ 0)
                int tm_wday;  // ���ڼ� (0 - 6��0 ����������)
                int tm_yday;  // һ���еĵڼ��� (0 - 365)
                int tm_isdst; // ����ʱ��־ (��ֵ��ʾ����ʱ��0 ��ʾ��ʹ������ʱ����ֵ��ʾ�޷�ȷ��)
        };*/
        localtime_s(&local_tm, &now_time);//localtime_s ���̰߳�ȫ�汾���ܹ��� time_t ʱ���ת��Ϊ�ṹ���ʽ���ꡢ�¡��ա�ʱ���֡���ȣ���
        char timebuf[20];
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &local_tm);
		//strftime �������ڽ�ʱ���ʽ��Ϊ�ַ�������һ��������Ŀ���ַ������������ڶ��������ǻ�������С�������������Ǹ�ʽ���ַ��������ĸ�������ʱ��ṹ�塣

        // ����ʱ�������Ϣ
        message += " (" + string(timebuf) + ")";

        if (message.find("@") == 0) {//�������ַ����״γ��ֵ�λ�ã��� 0 ��ʼ�������������δ�ҵ�ָ�������ַ������򷵻� std::string::npos��
            // ˽����Ϣ��ʽΪ @clientID message
            if (send(clifd, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
                cerr << "send error: " << WSAGetLastError() << endl;
                break;
            }
        }
        else {
            // Ⱥ����Ϣ�����ü�@
            if (send(clifd, ("@all " + message).c_str(), message.length() + 5, 0) == SOCKET_ERROR) {
                cerr << "send error: " << WSAGetLastError() << endl;
                break;
            }
        }

        if (send(clifd, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
            cerr << "send error: " << WSAGetLastError() << endl;
            break;
        }
		memset(sendbuf, 0, BUFSIZ);//memset �������ڽ�ָ�����ڴ���������Ϊָ����ֵ��ͨ�����ڳ�ʼ���ڴ�����
    }
    closesocket(clifd);
}

bool authenticate(SOCKET clifd) {
    string username, password;
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;

    string authStr = username + ":" + password;//����ؽ��䷢�͵�������������֤��
    if (send(clifd, authStr.c_str(), authStr.length(), 0) == SOCKET_ERROR) {
        cerr << "send error: " << WSAGetLastError() << endl;
        return false;
    }

    char recvbuf[BUFSIZ] = { 0 };
    int recvResult = recv(clifd, recvbuf, static_cast<int>(BUFSIZ - 1), 0);//����ת�� �����������ڽ�һ�����ʽ��ֵת��ΪĿ�����͡�
    if (recvResult > 0) {
        recvbuf[recvResult] = '\0';
        string response(recvbuf);//�� recvbuf ת��Ϊ string ���ͣ����ں����ıȽϲ�����response �洢�˷���������Ӧ��Ϣ��
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

    SOCKET clifd = createClientSocket("127.0.0.1", 12345); // ָ��������IP�Ͷ˿ں�
    if (clifd == INVALID_SOCKET) {
        cout << "Failed to create client socket" << endl;
        close_socket();
        return -1;
    }

    cout << "Connected to server!" << endl;

    if (!authenticate(clifd)) {
        cerr << "Authentication failed" << endl;
		closesocket(clifd);//�ر��׽���
		close_socket();//�ر������
        return -1;
    }

    // ������ͼ�߳�
    thread drawThread(drawChatWindow);

    thread recvThread(receiveMessages, clifd);
	thread sendThread(sendMessages, clifd);//������Ϣ���պͷ����̣߳��ֱ����ڽ��պͷ�����Ϣ��

    recvThread.join();
    sendThread.join();
    drawThread.join();//ȷ�� ���̵߳ȴ� ������ ���߳� ִ�����֮�󣬲��ܼ���ִ�����߳��еĺ������롣

    closesocket(clifd);
    close_socket();

    cout << "-----Client closed.-----" << endl;
    return 0;
}

