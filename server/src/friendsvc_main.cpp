#include "../../third_party/httplib/httplib.h"
#include "../include/utils/logger.h"
#include "../include/utils/database_manager.h"
#include "../include/services/friend_handler.h"
#include <iostream>

using namespace iuim::utils;
using namespace iuim::services;

int main() {
    try {
        // 初始化日志
        Logger::getInstance().logInfo("Starting FriendSVC microservice...");
        
        // 确保数据库已初始化
        DatabaseManager::getInstance();
        Logger::getInstance().logInfo("Database initialized successfully");
        
        // 创建HTTP服务器
        httplib::Server server;
        
        // 绑定路由
        server.Post("/add", handleFriendAdd);
        server.Post("/delete", handleFriendDelete);
        server.Post("/query", handleFriendQuery);
        
        // 启动服务器
        Logger::getInstance().logInfo("FriendSVC listening on port 50054");
        server.listen("0.0.0.0", 50054);
        
    } catch (const std::exception& e) {
        Logger::getInstance().logError("FriendSVC startup error: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}