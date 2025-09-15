#include "../../include/base/PlatformManager.h"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 初始化静态成员
PlatformManager* PlatformManager::instance = nullptr;

PlatformManager::PlatformManager() : networkServer(nullptr) {
    // 初始化网络服务器
    networkServer = new NetworkServer();
    
    // 设置消息处理函数
    networkServer->SetMessageHandler([this](const string& message, SOCKET clientSocket) {
        this->HandleMessage(message, clientSocket);
    });
}

PlatformManager::~PlatformManager() {
    // 停止网络服务器
    if (networkServer) {
        networkServer->Stop();
        delete networkServer;
    }
    
    // 释放用户资源
    for (auto& pair : allUsers) {
        delete pair.second;
    }
}

PlatformManager* PlatformManager::GetInstance() {
    if (instance == nullptr) {
        instance = new PlatformManager();
    }
    return instance;
}

bool PlatformManager::RegisterUser(UserBase* user) {
    if (user == nullptr) {
        return false;
    }
    
    string userID = user->GetID();
    
    // 检查用户是否已存在
    if (allUsers.find(userID) != allUsers.end()) {
        return false;
    }
    
    allUsers[userID] = user;
    return true;
}

UserBase* PlatformManager::GetUser(const string& userID) {
    auto it = allUsers.find(userID);
    if (it != allUsers.end()) {
        return it->second;
    }
    return nullptr;
}

bool PlatformManager::Login(const string& userID, const string& password) {
    UserBase* user = GetUser(userID);
    if (user == nullptr) {
        return false;
    }
    
    // 验证密码
    if (!user->CheckPassword(password)) {
        return false;
    }
    
    // 更新登录状态
    loginStatus[userID] = true;
    return true;
}

bool PlatformManager::Logout(const string& userID) {
    auto it = loginStatus.find(userID);
    if (it != loginStatus.end()) {
        loginStatus[userID] = false;
        return true;
    }
    return false;
}

bool PlatformManager::IsLoggedIn(const string& userID) const {
    auto it = loginStatus.find(userID);
    if (it != loginStatus.end()) {
        return it->second;
    }
    return false;
}

bool PlatformManager::StartServer(int port) {
    if (networkServer->Initialize()) {
        return networkServer->Start();
    }
    return false;
}

bool PlatformManager::StopServer() {
    return networkServer->Stop();
}

bool PlatformManager::SendMessageToClient(SOCKET clientSocket, const string& message) {
    return networkServer->SendMessage(clientSocket, message);
}

void PlatformManager::HandleMessage(const string& message, SOCKET clientSocket) {
    try {
        // 解析JSON消息
        json jsonMessage = json::parse(message);
        
        // 获取消息类型
        string type = jsonMessage["type"];
        
        // 根据消息类型处理
        if (type == "login") {
            string userID = jsonMessage["userID"];
            string password = jsonMessage["password"];
            
            bool success = Login(userID, password);
            
            // 构造响应
            json response;
            response["type"] = "login_response";
            response["success"] = success;
            
            if (success) {
                UserBase* user = GetUser(userID);
                response["nickname"] = user->GetNickname();
            }
            
            // 发送响应
            SendMessageToClient(clientSocket, response.dump());
        }
        else if (type == "logout") {
            string userID = jsonMessage["userID"];
            
            bool success = Logout(userID);
            
            // 构造响应
            json response;
            response["type"] = "logout_response";
            response["success"] = success;
            
            // 发送响应
            SendMessageToClient(clientSocket, response.dump());
        }
        // 可以添加更多消息类型的处理
    }
    catch (const exception& e) {
        cerr << "Error handling message: " << e.what() << endl;
    }
}

bool PlatformManager::SaveData() {
    // 简单实现，实际应用中应该使用SQLite
    return true;
}

bool PlatformManager::LoadData() {
    // 简单实现，实际应用中应该使用SQLite
    return true;
}