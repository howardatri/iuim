#include "../include/services/group_handler.h"
#include "../include/services/group_controller.h"
#include "../include/utils/database_manager.h"
#include "../include/utils/logger.h"
#include "../../third_party/httplib/httplib.h"
#include <iostream>

int main() {
    // 初始化日志
    iuim::utils::Logger::getInstance().logInfo("Starting GroupSVC with Object-Oriented Architecture...");
    
    // 初始化数据库
    try {
        iuim::utils::DatabaseManager::getInstance();
        iuim::utils::Logger::getInstance().logInfo("Database initialized successfully");
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Failed to initialize database: " + std::string(e.what()));
        return 1;
    }
    
    // 创建HTTP服务器
    httplib::Server server;
    
    // 设置路由 - 使用面向对象的GroupController
    server.Post("/join", iuim::services::GroupController::handleGroupJoin);
    server.Post("/quit", iuim::services::GroupController::handleGroupQuit);
    server.Post("/members", iuim::services::GroupController::handleGroupMembers);
    server.Post("/create", iuim::services::GroupController::handleGroupCreate);
    server.Post("/list", iuim::services::GroupController::handleGroupList);
    
    // 启动服务器
    iuim::utils::Logger::getInstance().logInfo("GroupSVC starting on port 50055 with OOP implementation");
    server.listen("0.0.0.0", 50055);
    
    return 0;
}