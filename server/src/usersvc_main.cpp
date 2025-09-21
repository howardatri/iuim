#include "../include/services/user_service.h"
#include "../include/utils/logger.h"
#include "../include/utils/database_manager.h"
#include <iostream>

int main() {
    // 初始化数据库单例
    iuim::utils::DatabaseManager::getInstance();
    iuim::utils::Logger::getInstance().logInfo("Database singleton initialized on startup.");
    
    iuim::utils::Logger::getInstance().logInfo("Starting IUIM UserSVC...");
    
    iuim::services::UserService userService;
    iuim::utils::Logger::getInstance().logInfo("Initializing UserService");
    
    userService.start();
    
    return 0;
}