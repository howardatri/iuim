#include "../../include/utils/NetworkServer.h"
#include <iostream>

NetworkServer::NetworkServer(int port) : port(port), isRunning(false), serverSocket(INVALID_SOCKET), clientThreads(nullptr) {}

NetworkServer::~NetworkServer() {
    Stop();
}

bool NetworkServer::Initialize() {
    // 初始化Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }

    // 创建套接字
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    // 绑定套接字
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    result = bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    // 监听
    result = listen(serverSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    return true;
}

bool NetworkServer::Start() {
    if (isRunning) {
        return false;
    }

    isRunning = true;
    listenThread = thread(&NetworkServer::ListenForClients, this);
    return true;
}

bool NetworkServer::Stop() {
    if (!isRunning) {
        return false;
    }

    isRunning = false;

    // 关闭所有客户端连接
    {
        lock_guard<mutex> lock(clientSocketsMutex);
        for (auto socket : clientSockets) {
            closesocket(socket);
        }
        clientSockets.clear();
    }

    // 关闭服务器套接字
    closesocket(serverSocket);
    
    // 等待监听线程结束
    if (listenThread.joinable()) {
        listenThread.join();
    }

    // 清理Winsock
    WSACleanup();

    return true;
}

void NetworkServer::ListenForClients() {
    while (isRunning) {
        // 接受客户端连接
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            if (isRunning) {
                std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            }
            continue;
        }

        // 添加到客户端列表
        {
            lock_guard<mutex> lock(clientSocketsMutex);
            clientSockets.push_back(clientSocket);
        }

        // 创建线程处理客户端
        thread clientThread(&NetworkServer::HandleClient, this, clientSocket);
        clientThread.detach();
    }
}

void NetworkServer::HandleClient(SOCKET clientSocket) {
    char buffer[4096];
    int bytesReceived;

    while (isRunning) {
        // 接收消息
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            // 客户端断开连接
            DisconnectClient(clientSocket);
            break;
        }

        // 处理消息
        buffer[bytesReceived] = '\0';
        string message(buffer);

        if (messageHandler) {
            messageHandler(message, clientSocket);
        }
    }
}

void NetworkServer::SetMessageHandler(MessageHandlerFunc handler) {
    messageHandler = handler;
}

bool NetworkServer::SendMessage(SOCKET clientSocket, const string& message) {
    int result = send(clientSocket, message.c_str(), message.length(), 0);
    return result != SOCKET_ERROR;
}

bool NetworkServer::BroadcastMessage(const string& message) {
    lock_guard<mutex> lock(clientSocketsMutex);
    bool success = true;

    for (auto socket : clientSockets) {
        if (!SendMessage(socket, message)) {
            success = false;
        }
    }

    return success;
}

bool NetworkServer::DisconnectClient(SOCKET clientSocket) {
    lock_guard<mutex> lock(clientSocketsMutex);

    // 从客户端列表中移除
    for (auto it = clientSockets.begin(); it != clientSockets.end(); ++it) {
        if (*it == clientSocket) {
            clientSockets.erase(it);
            closesocket(clientSocket);
            return true;
        }
    }

    return false;
}