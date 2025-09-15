#include <iostream>
#include <string>
#include "../include/base/PlatformManager.h"
#include "../include/base/UserBase.h"

using namespace std;

int main() {
    // 设置控制台UTF-8编码
    system("chcp 65001");
    
    cout << "=== IUIM服务端启动 ===" << endl;
    
    // 获取平台管理器实例
    PlatformManager* platformManager = PlatformManager::GetInstance();
    
    // 创建测试用户
    UserBase* testUser = new UserBase("test", "测试用户", "password");
    platformManager->RegisterUser(testUser);
    
    cout << "已注册测试用户: ID=test, 密码=password" << endl;
    
    // 启动服务器
    int port = 8888;
    if (platformManager->StartServer(port)) {
        cout << "服务器启动成功，监听端口: " << port << endl;
        cout << "等待客户端连接..." << endl;
        
        // 保持程序运行
        string command;
        while (true) {
            cout << "输入'exit'退出服务器: ";
            getline(cin, command);
            
            if (command == "exit") {
                break;
            }
        }
        
        // 停止服务器
        platformManager->StopServer();
    } else {
        cout << "服务器启动失败!" << endl;
    }
    
    cout << "=== IUIM服务端已关闭 ===" << endl;
    
    return 0;
}