#include "../include/services/group_handler.h"
#include "../include/utils/database_manager.h"
#include "../include/utils/logger.h"
#include "../../third_party/httplib/httplib.h"
#include <iostream>

int main() {
    // 初始化日志
    iuim::utils::Logger::getInstance().logInfo("Starting GroupSVC...");
    
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
    
    // 设置路由
    server.Post("/join", iuim::services::handleGroupJoin);
    server.Post("/quit", iuim::services::handleGroupQuit);
    server.Post("/members", iuim::services::handleGroupMembers);
    server.Post("/create", iuim::services::handleGroupCreate);
    server.Post("/list", iuim::services::handleGroupList);
    
    // 启动服务器
    iuim::utils::Logger::getInstance().logInfo("GroupSVC starting on port 50055");
    server.listen("0.0.0.0", 50055);
    
    return 0;
}