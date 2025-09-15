#ifndef PLATFORM_MANAGER_H
#define PLATFORM_MANAGER_H

#include <map>
#include <string>
#include "../utils/NetworkServer.h"
#include "UserBase.h"

using namespace std;

// 平台管理类（单例模式）
class PlatformManager {
private:
    static PlatformManager* instance;  // 单例实例
    
    map<string, UserBase*> allUsers;     // 所有用户
    map<string, bool> loginStatus;       // 登录状态
    
    NetworkServer* networkServer;        // 网络服务器
    
    // 私有构造函数
    PlatformManager();
    
    // 消息处理函数
    void HandleMessage(const string& message, SOCKET clientSocket);

public:
    // 获取单例实例
    static PlatformManager* GetInstance();
    
    // 析构函数
    ~PlatformManager();
    
    // 用户管理
    bool RegisterUser(UserBase* user);
    UserBase* GetUser(const string& userID);
    
    // 登录管理
    bool Login(const string& userID, const string& password);
    bool Logout(const string& userID);
    bool IsLoggedIn(const string& userID) const;
    
    // 网络服务
    bool StartServer(int port = 8888);
    bool StopServer();
    bool SendMessageToClient(SOCKET clientSocket, const string& message);
    
    // 数据持久化
    bool SaveData();
    bool LoadData();
};

#endif // PLATFORM_MANAGER_H