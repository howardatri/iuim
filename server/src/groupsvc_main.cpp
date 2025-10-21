#include "../include/services/group_handler.h"
#include "../include/services/group_controller.h"
#include "../include/utils/database_manager.h"
#include "../include/utils/group_db_manager.h"
#include "../include/utils/logger.h"
#include "../../third_party/httplib/httplib.h"
#include <iostream>

int main() {
    // 初始化日志
    iuim::utils::Logger::getInstance().logInfo("Starting Enhanced GroupSVC with Object-Oriented Architecture...");
    
    // 初始化数据库
    try {
        iuim::utils::DatabaseManager::getInstance();
        iuim::utils::Logger::getInstance().logInfo("Database initialized successfully");
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Failed to initialize database: " + std::string(e.what()));
        return 1;
    }
    
    // 初始化群组数据库管理器
    try {
        iuim::utils::GroupDBManager::getInstance();
        iuim::utils::Logger::getInstance().logInfo("GroupDBManager initialized successfully");
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Failed to initialize GroupDBManager: " + std::string(e.what()));
        return 1;
    }
    
    // 创建HTTP服务器
    httplib::Server server;
    
    // 设置基础路由 - 使用面向对象的GroupController
    server.Post("/join", iuim::services::GroupController::handleGroupJoin);
    server.Post("/quit", iuim::services::GroupController::handleGroupQuit);
    server.Post("/members", iuim::services::GroupController::handleGroupMembers);
    server.Post("/create", iuim::services::GroupController::handleGroupCreate);
    server.Post("/list", iuim::services::GroupController::handleGroupList);
    
    // 绑定新的增强群组功能路由
    server.Post("/group_settings", iuim::services::GroupController::handleGetGroupSettings);
    server.Post("/update_group_settings", iuim::services::GroupController::handleUpdateGroupSettings);
    server.Post("/change_group_type", iuim::services::GroupController::handleChangeGroupType);
    server.Post("/set_member_role", iuim::services::GroupController::handleSetMemberRole);
    server.Post("/get_member_roles", iuim::services::GroupController::handleGetMemberRoles);
    server.Post("/apply_join_group", iuim::services::GroupController::handleApplyJoinGroup);
    server.Post("/invite_join_group", iuim::services::GroupController::handleInviteJoinGroup);
    server.Post("/free_join_topic", iuim::services::GroupController::handleFreeJoinTopic);
    server.Post("/get_current_user_role", iuim::services::GroupController::handleGetCurrentUserRole);
    
    // 启动服务器
    iuim::utils::Logger::getInstance().logInfo("Enhanced GroupSVC starting on port 50055 with advanced group management features");
    server.listen("0.0.0.0", 50055);
    
    return 0;
}