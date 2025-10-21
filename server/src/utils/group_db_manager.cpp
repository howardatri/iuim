#include "../../include/utils/group_db_manager.h"
#include "../../include/utils/logger.h"
#include "../../include/utils/database_manager.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>

namespace iuim {
namespace utils {

// // 辅助函数：转义JSON字符串中的特殊字符
// std::string escapeJsonString(const std::string& input) {
//     std::ostringstream oss;
//     for (char c : input) {
//         switch (c) {
//             case '"':  oss << "\\\""; break;
//             case '\\': oss << "\\\\"; break;
//             case '\b': oss << "\\b";  break;
//             case '\f': oss << "\\f";  break;
//             case '\n': oss << "\\n";  break;
//             case '\r': oss << "\\r";  break;
//             case '\t': oss << "\\t";  break;
//             default:
//                 if ('\x00' <= c && c <= '\x1f') {
//                     oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
//                 } else {
//                     oss << c;
//                 }
//         }
//     }
//     return oss.str();
// }

// 获取单例实例
GroupDBManager& GroupDBManager::getInstance() {
    static GroupDBManager instance;
    return instance;
}

GroupDBManager::GroupDBManager(const std::string& dbPath)
    : db_(nullptr), dbPath_(dbPath) {
    // 使用主DatabaseManager的数据库连接
    db_ = DatabaseManager::getInstance().getDatabase();
    if (!db_) {
        std::string errorMsg = "Failed to get database connection from DatabaseManager";
        Logger::getInstance().logError(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    
    Logger::getInstance().logInfo("GroupDBManager initialized successfully with shared database connection");
}

GroupDBManager::~GroupDBManager() {
    // 不关闭共享的数据库连接，由DatabaseManager管理
    Logger::getInstance().logInfo("GroupDBManager destroyed");
}



void GroupDBManager::setServiceDefaultSettings(int serviceId, int& groupType, int& joinMethod, 
                                             int& allowSubgroups, int& adminSystem, int& maxMembers) {
    switch(serviceId) {
        case 1: // QQ
            groupType = 0;
            joinMethod = 0; // 申请加入
            allowSubgroups = 1; // 允许子群
            adminSystem = 2; // 完整管理员制度
            maxMembers = 1000;
            break;
        case 2: // 微信
            groupType = 1;
            joinMethod = 1; // 推荐加入
            allowSubgroups = 0; // 不允许子群
            adminSystem = 0; // 无管理员制度
            maxMembers = 500;
            break;
        case 3: // 微博
            groupType = 2;
            joinMethod = 2; // 自由加入
            allowSubgroups = 0; // 不允许子群
            adminSystem = 1; // 简单管理
            maxMembers = 10000;
            break;
        default:
            // 默认使用QQ配置
            groupType = 0;
            joinMethod = 0;
            allowSubgroups = 1;
            adminSystem = 2;
            maxMembers = 500;
    }
}

bool GroupDBManager::getGroupSettings(int groupId, int serviceId, std::string& jsonResult) {
    // 首先获取群组基本信息
    const char* groupInfoSql = R"(
        SELECT g.group_name, g.description, u.username as creator_name
        FROM Groups g
        LEFT JOIN Users u ON g.creator_id = u.id
        WHERE g.group_id = ?
    )";
    
    sqlite3_stmt* groupStmt;
    int rc = sqlite3_prepare_v2(db_, groupInfoSql, -1, &groupStmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare group info statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(groupStmt, 1, groupId);
    
    std::string groupName, description, creatorName;
    rc = sqlite3_step(groupStmt);
    if (rc == SQLITE_ROW) {
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(groupStmt, 0));
        const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(groupStmt, 1));
        const char* creator = reinterpret_cast<const char*>(sqlite3_column_text(groupStmt, 2));
        
        groupName = name ? name : "";
        description = desc ? desc : "";
        creatorName = creator ? creator : "";
    } else {
        sqlite3_finalize(groupStmt);
        Logger::getInstance().logError("Group not found: " + std::to_string(groupId));
        return false;
    }
    sqlite3_finalize(groupStmt);
    
    // 然后获取群组设置信息
    const char* settingsSql = R"(
        SELECT group_type, join_method, allow_subgroups, admin_system, max_members 
        FROM GroupSettings 
        WHERE group_id = ? AND service_id = ?
    )";
    
    sqlite3_stmt* settingsStmt;
    rc = sqlite3_prepare_v2(db_, settingsSql, -1, &settingsStmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare getGroupSettings statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(settingsStmt, 1, groupId);
    sqlite3_bind_int(settingsStmt, 2, serviceId);
    
    rc = sqlite3_step(settingsStmt);
    if (rc == SQLITE_ROW) {
        int groupType = sqlite3_column_int(settingsStmt, 0);
        int joinMethod = sqlite3_column_int(settingsStmt, 1);
        int allowSubgroups = sqlite3_column_int(settingsStmt, 2);
        int adminSystem = sqlite3_column_int(settingsStmt, 3);
        int maxMembers = sqlite3_column_int(settingsStmt, 4);
        
        std::ostringstream oss;
        oss << "{"
            << "\"group_name\":\"" << groupName << "\","
            << "\"description\":\"" << description << "\","
            << "\"creator_name\":\"" << creatorName << "\","
            << "\"group_type\":" << groupType << ","
            << "\"join_method\":" << joinMethod << ","
            << "\"allow_subgroups\":" << allowSubgroups << ","
            << "\"admin_system\":" << adminSystem << ","
            << "\"max_members\":" << maxMembers
            << "}";
        jsonResult = oss.str();
        
        sqlite3_finalize(settingsStmt);
        Logger::getInstance().logInfo("Retrieved group settings for group_id: " + std::to_string(groupId) + ", service_id: " + std::to_string(serviceId));
        return true;
    } else if (rc == SQLITE_DONE) {
        // 如果没有找到设置，创建默认设置
        sqlite3_finalize(settingsStmt);
        
        int groupType, joinMethod, allowSubgroups, adminSystem, maxMembers;
        setServiceDefaultSettings(serviceId, groupType, joinMethod, allowSubgroups, adminSystem, maxMembers);
        
        if (updateGroupSettings(groupId, serviceId, groupType, joinMethod, allowSubgroups, adminSystem, maxMembers)) {
            std::ostringstream oss;
            oss << "{"
                << "\"group_name\":\"" << groupName << "\","
                << "\"description\":\"" << description << "\","
                << "\"creator_name\":\"" << creatorName << "\","
                << "\"group_type\":" << groupType << ","
                << "\"join_method\":" << joinMethod << ","
                << "\"allow_subgroups\":" << allowSubgroups << ","
                << "\"admin_system\":" << adminSystem << ","
                << "\"max_members\":" << maxMembers
                << "}";
            jsonResult = oss.str();
            return true;
        }
        return false;
    } else {
        Logger::getInstance().logError("Failed to execute getGroupSettings: " + std::string(sqlite3_errmsg(db_)));
        sqlite3_finalize(settingsStmt);
        return false;
    }
}

bool GroupDBManager::updateGroupSettings(int groupId, int serviceId, int groupType, int joinMethod, 
                                        int allowSubgroups, int adminSystem, int maxMembers) {
    const char* sql = R"(
        INSERT OR REPLACE INTO GroupSettings 
        (group_id, service_id, group_type, join_method, allow_subgroups, admin_system, max_members) 
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare updateGroupSettings statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, groupId);
    sqlite3_bind_int(stmt, 2, serviceId);
    sqlite3_bind_int(stmt, 3, groupType);
    sqlite3_bind_int(stmt, 4, joinMethod);
    sqlite3_bind_int(stmt, 5, allowSubgroups);
    sqlite3_bind_int(stmt, 6, adminSystem);
    sqlite3_bind_int(stmt, 7, maxMembers);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        Logger::getInstance().logInfo("Updated group settings for group_id: " + std::to_string(groupId) + ", service_id: " + std::to_string(serviceId));
        return true;
    } else {
        Logger::getInstance().logError("Failed to update group settings: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
}

// 在 GroupDBManager::changeGroupType 方法中添加群组服务迁移逻辑
bool GroupDBManager::changeGroupType(int groupId, int sourceServiceId, int targetServiceId) {
    // 首先检查目标服务中是否已存在该群组
    const char* checkSql = "SELECT 1 FROM GroupMembers WHERE group_id = ? AND service_id = ? LIMIT 1";
    sqlite3_stmt* checkStmt;
    int rc = sqlite3_prepare_v2(db_, checkSql, -1, &checkStmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare check statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(checkStmt, 1, groupId);
    sqlite3_bind_int(checkStmt, 2, targetServiceId);
    rc = sqlite3_step(checkStmt);
    bool existsInTarget = (rc == SQLITE_ROW);
    sqlite3_finalize(checkStmt);
    
    if (existsInTarget) {
        Logger::getInstance().logError("Group already exists in target service: " + std::to_string(groupId));
        return false;
    }
    
    // 首先更新Groups表的service_id（最重要的步骤）
    const char* updateGroupsSql = R"(
        UPDATE Groups 
        SET service_id = ? 
        WHERE group_id = ? AND service_id = ?
    )";
    
    sqlite3_stmt* updateGroupsStmt;
    rc = sqlite3_prepare_v2(db_, updateGroupsSql, -1, &updateGroupsStmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare update Groups statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(updateGroupsStmt, 1, targetServiceId);
    sqlite3_bind_int(updateGroupsStmt, 2, groupId);
    sqlite3_bind_int(updateGroupsStmt, 3, sourceServiceId);
    
    rc = sqlite3_step(updateGroupsStmt);
    sqlite3_finalize(updateGroupsStmt);
    
    if (rc != SQLITE_DONE) {
        Logger::getInstance().logError("Failed to update Groups table: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    // 迁移群组成员到新服务
    const char* migrateMembersSql = R"(
        UPDATE GroupMembers 
        SET service_id = ? 
        WHERE group_id = ? AND service_id = ?
    )";
    
    sqlite3_stmt* migrateStmt;
    rc = sqlite3_prepare_v2(db_, migrateMembersSql, -1, &migrateStmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare migrate members statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(migrateStmt, 1, targetServiceId);
    sqlite3_bind_int(migrateStmt, 2, groupId);
    sqlite3_bind_int(migrateStmt, 3, sourceServiceId);
    
    rc = sqlite3_step(migrateStmt);
    sqlite3_finalize(migrateStmt);
    
    if (rc != SQLITE_DONE) {
        Logger::getInstance().logError("Failed to migrate group members: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    // 迁移群组角色到新服务
    const char* migrateRolesSql = R"(
        UPDATE GroupMemberRoles 
        SET service_id = ? 
        WHERE group_id = ? AND service_id = ?
    )";
    
    sqlite3_stmt* rolesStmt;
    rc = sqlite3_prepare_v2(db_, migrateRolesSql, -1, &rolesStmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare migrate roles statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(rolesStmt, 1, targetServiceId);
    sqlite3_bind_int(rolesStmt, 2, groupId);
    sqlite3_bind_int(rolesStmt, 3, sourceServiceId);
    
    rc = sqlite3_step(rolesStmt);
    sqlite3_finalize(rolesStmt);
    
    if (rc != SQLITE_DONE) {
        Logger::getInstance().logError("Failed to migrate group roles: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    // 获取新的默认设置
    int groupType, joinMethod, allowSubgroups, adminSystem, maxMembers;
    setServiceDefaultSettings(targetServiceId, groupType, joinMethod, allowSubgroups, adminSystem, maxMembers);
    
    // 更新群组设置到新服务
    if (!updateGroupSettings(groupId, targetServiceId, groupType, joinMethod, allowSubgroups, adminSystem, maxMembers)) {
        Logger::getInstance().logError("Failed to update group settings for target service");
        return false;
    }
    
    // 删除原服务的群组设置（可选，根据需求决定是否保留）
    const char* deleteOldSettingsSql = "DELETE FROM GroupSettings WHERE group_id = ? AND service_id = ?";
    sqlite3_stmt* deleteStmt;
    rc = sqlite3_prepare_v2(db_, deleteOldSettingsSql, -1, &deleteStmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(deleteStmt, 1, groupId);
        sqlite3_bind_int(deleteStmt, 2, sourceServiceId);
        sqlite3_step(deleteStmt);
        sqlite3_finalize(deleteStmt);
    }
    
    Logger::getInstance().logInfo("Successfully changed group type for group_id: " + std::to_string(groupId) + 
                                " from service_id: " + std::to_string(sourceServiceId) + 
                                " to service_id: " + std::to_string(targetServiceId));
    return true;
}

bool GroupDBManager::setMemberRole(int groupId, int userId, int serviceId, int roleType) {
    const char* sql = R"(
        INSERT OR REPLACE INTO GroupMemberRoles 
        (group_id, user_id, service_id, role_type) 
        VALUES (?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare setMemberRole statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, groupId);
    sqlite3_bind_int(stmt, 2, userId);
    sqlite3_bind_int(stmt, 3, serviceId);
    sqlite3_bind_int(stmt, 4, roleType);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        Logger::getInstance().logInfo("Set member role for user_id: " + std::to_string(userId) + 
                                    " in group_id: " + std::to_string(groupId) + 
                                    " to role_type: " + std::to_string(roleType));
        return true;
    } else {
        Logger::getInstance().logError("Failed to set member role: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
}

bool GroupDBManager::getMemberRoles(int groupId, int serviceId, std::string& jsonResult) {
    const char* sql = R"(
        SELECT gmr.user_id, gmr.role_type, u.username, u.nickname 
        FROM GroupMemberRoles gmr
        LEFT JOIN Users u ON gmr.user_id = u.id
        WHERE gmr.group_id = ? AND gmr.service_id = ?
        ORDER BY gmr.role_type DESC, gmr.assigned_time ASC
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare getMemberRoles statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, groupId);
    sqlite3_bind_int(stmt, 2, serviceId);
    
    std::ostringstream oss;
    oss << "{\"members\":[";
    
    bool first = true;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (!first) oss << ",";
        first = false;
        
        int userId = sqlite3_column_int(stmt, 0);
        int roleType = sqlite3_column_int(stmt, 1);
        std::string username = sqlite3_column_text(stmt, 2) ? (char*)sqlite3_column_text(stmt, 2) : "";
        std::string nickname = sqlite3_column_text(stmt, 3) ? (char*)sqlite3_column_text(stmt, 3) : "";
        
        oss << "{"
            << "\"user_id\":" << userId << ","
            << "\"role_type\":" << roleType << ","
            << "\"username\":\"" << escapeJsonString(username) << "\","
            << "\"nickname\":\"" << escapeJsonString(nickname) << "\""
            << "}";
    }
    
    oss << "]}";
    jsonResult = oss.str();
    
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        Logger::getInstance().logInfo("Retrieved member roles for group_id: " + std::to_string(groupId) + ", service_id: " + std::to_string(serviceId));
        return true;
    } else {
        Logger::getInstance().logError("Failed to get member roles: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
}

bool GroupDBManager::getCurrentUserRole(int groupId, int userId, std::string& jsonResult) {
    const char* sql = R"(
        SELECT gmr.role_type 
        FROM GroupMemberRoles gmr
        WHERE gmr.group_id = ? AND gmr.user_id = ?
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare getCurrentUserRole statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, groupId);
    sqlite3_bind_int(stmt, 2, userId);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int roleType = sqlite3_column_int(stmt, 0);
        
        std::ostringstream oss;
        oss << "{"
            << "\"code\":0,"
            << "\"message\":\"success\","
            << "\"data\":{"
            << "\"role_type\":" << roleType
            << "}"
            << "}";
        jsonResult = oss.str();
        
        sqlite3_finalize(stmt);
        Logger::getInstance().logInfo("Retrieved user role: " + std::to_string(roleType) + " for user_id: " + std::to_string(userId) + " in group_id: " + std::to_string(groupId));
        return true;
    } else if (rc == SQLITE_DONE) {
        // 用户不在群组中或没有角色记录，返回访客角色
        std::ostringstream oss;
        oss << "{"
            << "\"code\":0,"
            << "\"message\":\"success\","
            << "\"data\":{"
            << "\"role_type\":3"  // RoleGuest = 3
            << "}"
            << "}";
        jsonResult = oss.str();
        
        sqlite3_finalize(stmt);
        Logger::getInstance().logInfo("User not found in group, returning guest role for user_id: " + std::to_string(userId) + " in group_id: " + std::to_string(groupId));
        return true;
    } else {
        Logger::getInstance().logError("Failed to execute getCurrentUserRole: " + std::string(sqlite3_errmsg(db_)));
        sqlite3_finalize(stmt);
        return false;
    }
}

bool GroupDBManager::applyJoinGroup(int groupId, int userId, int serviceId, const std::string& reason) {
    // 检查是否为QQ群（只有QQ群允许申请加入）
    std::string settingsJson;
    if (!getGroupSettings(groupId, serviceId, settingsJson)) {
        Logger::getInstance().logError("Failed to get group settings for apply join");
        return false;
    }
    
    // 这里应该检查join_method是否为0（申请加入），简化实现
    // 实际应该解析JSON或直接查询数据库
    
    // 检查用户是否已在群组中
    if (isUserInGroup(userId, groupId, serviceId)) {
        Logger::getInstance().logInfo("User " + std::to_string(userId) + " is already in group " + std::to_string(groupId));
        return false;
    }
    
    // 这里应该创建申请记录，简化实现直接加入群组
    // 实际项目中应该有申请审核流程
    const char* sql = R"(
        INSERT OR IGNORE INTO GroupMembers (user_id, group_id, service_id, join_time) 
        VALUES (?, ?, ?, CURRENT_TIMESTAMP)
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare applyJoinGroup statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, groupId);
    sqlite3_bind_int(stmt, 3, serviceId);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        // 设置为普通成员角色
        setMemberRole(groupId, userId, serviceId, 0);
        Logger::getInstance().logInfo("User " + std::to_string(userId) + " applied to join group " + std::to_string(groupId) + " with reason: " + reason);
        return true;
    } else {
        Logger::getInstance().logError("Failed to apply join group: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
}

bool GroupDBManager::inviteJoinGroup(int groupId, int inviterId, int inviteeId, int serviceId) {
    // 检查邀请者是否在群组中
    if (!isUserInGroup(inviterId, groupId, serviceId)) {
        Logger::getInstance().logError("Inviter " + std::to_string(inviterId) + " is not in group " + std::to_string(groupId));
        return false;
    }
    
    // 检查被邀请者是否已在群组中
    if (isUserInGroup(inviteeId, groupId, serviceId)) {
        Logger::getInstance().logInfo("Invitee " + std::to_string(inviteeId) + " is already in group " + std::to_string(groupId));
        return false;
    }
    
    // 直接加入群组（简化实现，实际应该有邀请确认流程）
    const char* sql = R"(
        INSERT OR IGNORE INTO GroupMembers (user_id, group_id, service_id, join_time) 
        VALUES (?, ?, ?, CURRENT_TIMESTAMP)
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare inviteJoinGroup statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, inviteeId);
    sqlite3_bind_int(stmt, 2, groupId);
    sqlite3_bind_int(stmt, 3, serviceId);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        // 设置为普通成员角色
        setMemberRole(groupId, inviteeId, serviceId, 0);
        Logger::getInstance().logInfo("User " + std::to_string(inviterId) + " invited user " + std::to_string(inviteeId) + " to join group " + std::to_string(groupId));
        return true;
    } else {
        Logger::getInstance().logError("Failed to invite join group: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
}

bool GroupDBManager::freeJoinTopic(int groupId, int userId, int serviceId) {
    // 检查用户是否已在群组中
    if (isUserInGroup(userId, groupId, serviceId)) {
        Logger::getInstance().logInfo("User " + std::to_string(userId) + " is already in group " + std::to_string(groupId));
        return false;
    }
    
    // 直接加入超话
    const char* sql = R"(
        INSERT OR IGNORE INTO GroupMembers (user_id, group_id, service_id, join_time) 
        VALUES (?, ?, ?, CURRENT_TIMESTAMP)
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare freeJoinTopic statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, groupId);
    sqlite3_bind_int(stmt, 3, serviceId);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        // 设置为普通成员角色
        setMemberRole(groupId, userId, serviceId, 0);
        Logger::getInstance().logInfo("User " + std::to_string(userId) + " freely joined topic " + std::to_string(groupId));
        return true;
    } else {
        Logger::getInstance().logError("Failed to free join topic: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
}

bool GroupDBManager::isGroupExists(int groupId) {
    const char* sql = "SELECT 1 FROM Groups WHERE group_id = ? LIMIT 1";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare isGroupExists statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, groupId);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_ROW);
}

bool GroupDBManager::isUserInGroup(int userId, int groupId, int serviceId) {
    const char* sql = "SELECT 1 FROM GroupMembers WHERE user_id = ? AND group_id = ? AND service_id = ? LIMIT 1";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare isUserInGroup statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, groupId);
    sqlite3_bind_int(stmt, 3, serviceId);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_ROW);
}

bool GroupDBManager::getUserInfo(int userId, std::string& username, std::string& nickname) {
    const char* sql = "SELECT username, nickname FROM Users WHERE id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare getUserInfo statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        username = sqlite3_column_text(stmt, 0) ? (char*)sqlite3_column_text(stmt, 0) : "";
        nickname = sqlite3_column_text(stmt, 1) ? (char*)sqlite3_column_text(stmt, 1) : "";
        sqlite3_finalize(stmt);
        return true;
    } else {
        sqlite3_finalize(stmt);
        return false;
    }
}

} // namespace utils
} // namespace iuim