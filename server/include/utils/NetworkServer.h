#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>

using namespace std;

// 服务端网络通信类
class NetworkServer {
private:
    SOCKET serverSocket;  // 服务器套接字
    bool isRunning;       // 服务器运行状态
    int port;             // 监听端口
    
    // 客户端连接管理
    vector<SOCKET> clientSockets;
    mutex clientSocketsMutex;
    
    // 消息处理回调函数
    typedef function<void(const string&, SOCKET)> MessageHandlerFunc;
    MessageHandlerFunc messageHandler;
    
    // 线程
    thread listenThread;
    thread* clientThreads;

    // 私有方法
    void ListenForClients();
    void HandleClient(SOCKET clientSocket);

public:
    NetworkServer(int port = 8888);
    ~NetworkServer();
    
    // 服务器控制
    bool Initialize();
    bool Start();
    bool Stop();
    
    // 消息处理
    void SetMessageHandler(MessageHandlerFunc handler);
    bool SendMessage(SOCKET clientSocket, const string& message);
    bool BroadcastMessage(const string& message);
    
    // 客户端管理
    bool DisconnectClient(SOCKET clientSocket);
};

#endif // NETWORK_SERVER_H