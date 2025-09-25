#include "../include/services/service_handler.h"
#include "../include/utils/database_manager.h"
#include "../include/utils/logger.h"
#include <iostream>

using namespace iuim::utils;
using namespace iuim::services;

int main() {
    // 初始化日志
    Logger::getInstance().logInfo("Starting ServiceSVC...");
    
    try {
        // 确保数据库已初始化
        DatabaseManager::getInstance();
        Logger::getInstance().logInfo("Database initialized successfully");
        
        // 创建HTTP服务器
        httplib::Server server;
        
        // 注册路由
        server.Post("/activate", handleActivate);
        server.Post("/deactivate", handleDeactivate);
        server.Post("/query_user_services", handleQueryUserServices);
        
        // 添加健康检查接口
        server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        });
        
        // 启动服务器
        Logger::getInstance().logInfo("ServiceSVC starting on port 50056...");
        server.listen("0.0.0.0", 50056);
    } catch (const std::exception& e) {
        Logger::getInstance().logError("Failed to start ServiceSVC: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}