#include "../../third_party/httplib/httplib.h"
#include "../include/utils/logger.h"
#include "../include/utils/database_manager.h"
#include "../include/services/friend_handler.h"
#include "../include/services/friend_controller.h"
#include <iostream>

using namespace iuim::utils;
using namespace iuim::services;

int main() {
    try {
        // 初始化日志
        Logger::getInstance().logInfo("Starting FriendSVC with Object-Oriented Wrapper...");
        
        // 确保数据库已初始化
        DatabaseManager::getInstance();
        Logger::getInstance().logInfo("Database initialized successfully");
        
        // 创建HTTP服务器
        httplib::Server server;
        
        // 绑定路由 - 使用面向对象的FriendController包裹器
        server.Post("/add", FriendController::handleFriendAdd);
        server.Post("/delete", FriendController::handleFriendDelete);
        server.Post("/query", FriendController::handleFriendQuery);

        // 新增：用户搜索功能
        server.Post("/search", FriendController::handleUserSearch);
        
        // 新增：跨服务好友功能
        server.Post("/common", FriendController::handleQueryCommonFriends);
        server.Post("/cross-service", FriendController::handleQueryCrossServiceFriends);
        
        // 启动服务器
        Logger::getInstance().logInfo("FriendSVC listening on port 50054 with OOP wrapper");
        server.listen("0.0.0.0", 50054);
        
    } catch (const std::exception& e) {
        Logger::getInstance().logError("FriendSVC startup error: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}