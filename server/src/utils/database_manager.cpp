#include "../../include/utils/database_manager.h"
#include "../../include/utils/logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <string>

namespace iuim {
namespace utils {

    // 新增的辅助函数：用于转义JSON字符串中的特殊字符
std::string escapeJsonString(const std::string& input) {
    std::ostringstream oss;
    for (char c : input) {
        switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b";  break;
            case '\f': oss << "\\f";  break;
            case '\n': oss << "\\n";  break;
            case '\r': oss << "\\r";  break;
            case '\t': oss << "\\t";  break;
            default:
                if ('\x00' <= c && c <= '\x1f') {
                    // 对于其他控制字符，使用unicode转义
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                } else {
                    oss << c;
                }
        }
    }
    return oss.str();
}

// 获取单例实例
DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

// 简单的SHA256哈希实现
std::string DatabaseManager::hashPassword(const std::string& password) {
    // 实际项目中应使用更安全的哈希库，这里为简化实现
    std::stringstream ss;
    // 简单哈希：将每个字符的ASCII值相加，然后转为十六进制
    int hash = 0;
    for (char c : password) {
        hash = (hash * 31 + c) % 0xFFFFFF;
    }
    ss << std::hex << std::setfill('0') << std::setw(8) << hash;
    return ss.str();
}

DatabaseManager::DatabaseManager(const std::string& dbPath)
    : db_(nullptr), dbPath_(dbPath) {
    // 在构造函数中执行初始化
    if (!initialize()) {
        std::string errorMsg = "Failed to initialize database";
        Logger::getInstance().logError(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    
    // 创建用户表
    if (!createUserTable()) {
        std::string errorMsg = "Failed to create user table";
        Logger::getInstance().logError(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    
    // 创建服务表和用户服务关系表
    if (!createServiceTables()) {
        std::string errorMsg = "Failed to create service tables";
        Logger::getInstance().logError(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    // 创建社交相关表
    if (!createSocialTables()) {
        std::string errorMsg = "Failed to create social tables";
        Logger::getInstance().logError(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    
    // 创建消息相关表
    if (!createMessageTables()) {
        std::string errorMsg = "Failed to create message tables";
        Logger::getInstance().logError(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    
    // 创建群组相关表
    if (!createGroupTables()) {
        std::string errorMsg = "Failed to create group tables";
        Logger::getInstance().logError(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    
    Logger::getInstance().logInfo("All database tables initialized successfully");
}



DatabaseManager::~DatabaseManager() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool DatabaseManager::initialize() {
    // 确保数据目录存在
    std::filesystem::path dir = std::filesystem::path(dbPath_).parent_path();
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }

    // 打开数据库连接
    int rc = sqlite3_open(dbPath_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Cannot open database: " + std::string(sqlite3_errmsg(db_));
        Logger::getInstance().logError(errorMsg);
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    iuim::utils::Logger::getInstance().logInfo("Database connection opened successfully: " + dbPath_);
    
    // 创建用户表
    return createUserTable();
}

bool DatabaseManager::createUserTable() {
    if (!db_) return false;

    const char* sql = 
        "CREATE TABLE IF NOT EXISTS Users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE,"
        "password TEXT NOT NULL,"
        "nickname TEXT,"
        "email TEXT,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ");";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::string errorMsg = "SQL error: " + std::string(errMsg);
        iuim::utils::Logger::getInstance().logError(errorMsg);
        sqlite3_free(errMsg);
        return false;
    }
    
    iuim::utils::Logger::getInstance().logInfo("User table created or already exists");
    return true;
}

bool DatabaseManager::registerUser(const std::string& username, const std::string& password, 
                                  const std::string& nickname, const std::string& email) {
    if (!db_) {
        iuim::utils::Logger::getInstance().logError("Database not initialized");
        return false;
    }

    // 检查用户是否已存在
    if (isUserExists(username)) {
        iuim::utils::Logger::getInstance().logInfo("Registration failed: Username already exists: " + username);
        return false;
    }
    
    iuim::utils::Logger::getInstance().logInfo("Registering new user: " + username);

    // 哈希密码
    std::string hashedPassword = hashPassword(password);

    // 准备SQL语句
    const char* sql = 
        "INSERT INTO Users (username, password, nickname, email) VALUES (?, ?, ?, ?);";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    // 绑定参数
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, nickname.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, email.c_str(), -1, SQLITE_STATIC);
    
    // 执行SQL
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool DatabaseManager::verifyLogin(const std::string& username, const std::string& password, 
                                 std::string& nickname, std::string& email) {
    if (!db_) return false;

    // 哈希密码
    std::string hashedPassword = hashPassword(password);

    // 准备SQL语句
    const char* sql = 
        "SELECT nickname, email FROM Users WHERE username = ? AND password = ?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    // 绑定参数
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
    
    // 执行SQL
    rc = sqlite3_step(stmt);
    
    bool success = false;
    if (rc == SQLITE_ROW) {
        // 获取结果
        const unsigned char* nick = sqlite3_column_text(stmt, 0);
        const unsigned char* mail = sqlite3_column_text(stmt, 1);
        
        if (nick) nickname = reinterpret_cast<const char*>(nick);
        if (mail) email = reinterpret_cast<const char*>(mail);
        
        success = true;
    }
    
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::isUserExists(const std::string& username) {
    if (!db_) return false;

    // 准备SQL语句
    const char* sql = "SELECT 1 FROM Users WHERE username = ?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    // 绑定参数
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    // 执行SQL
    rc = sqlite3_step(stmt);
    bool exists = (rc == SQLITE_ROW);
    
    sqlite3_finalize(stmt);
    return exists;
}

bool DatabaseManager::createServiceTables() {
    if (!db_) return false;

    // 创建Services表 - 按照设计文档4.2.2节添加description字段
    const char* sqlServices = 
        "CREATE TABLE IF NOT EXISTS Services ("
        "id INTEGER PRIMARY KEY,"
        "name TEXT NOT NULL UNIQUE,"
        "description TEXT"
        ");";

    // 创建UserServices表 - 按照设计文档4.2.3节添加service_user_id, activated, activate_time字段
    const char* sqlUserServices = 
        "CREATE TABLE IF NOT EXISTS UserServices ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "service_id INTEGER NOT NULL,"
        "service_user_id TEXT,"
        "activated INTEGER DEFAULT 0,"
        "activate_time TEXT,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "UNIQUE(user_id, service_id),"
        "FOREIGN KEY(user_id) REFERENCES Users(id),"
        "FOREIGN KEY(service_id) REFERENCES Services(id)"
        ");";

    // 插入初始服务数据 - 添加description字段
    const char* sqlInsertServices = 
        "INSERT OR IGNORE INTO Services (id, name, description) VALUES "
        "(1, 'QQ', '经典即时通讯服务'), (2, '微信', '现代社交生活方式'),(3, '微博', '分享生活点滴');";

    char* errMsg = nullptr;
    
    // 执行创建Services表
    int rc = sqlite3_exec(db_, sqlServices, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "SQL error creating Services table: " + std::string(errMsg);
        Logger::getInstance().logError(errorMsg);
        sqlite3_free(errMsg);
        return false;
    }
    
    // 执行创建UserServices表
    rc = sqlite3_exec(db_, sqlUserServices, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "SQL error creating UserServices table: " + std::string(errMsg);
        Logger::getInstance().logError(errorMsg);
        sqlite3_free(errMsg);
        return false;
    }
    
    // 检查Services表是否为空，如果为空则插入初始数据
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM Services", -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "SQL error checking Services table: " + std::string(sqlite3_errmsg(db_));
        Logger::getInstance().logError(errorMsg);
        return false;
    }
    
    rc = sqlite3_step(stmt);
    int count = 0;
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    
    // 如果表为空，插入初始服务数据
    if (count == 0) {
        rc = sqlite3_exec(db_, sqlInsertServices, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::string errorMsg = "SQL error inserting initial services: " + std::string(errMsg);
            Logger::getInstance().logError(errorMsg);
            sqlite3_free(errMsg);
            return false;
        }
        Logger::getInstance().logInfo("Initial services data inserted successfully");
    }
    
    Logger::getInstance().logInfo("Service tables created successfully");
    return true;
}

bool DatabaseManager::createSocialTables() {
    if (!db_) return false;

    // 创建好友表 - 按照设计文档4.2.4节
    const char* sqlFriends = 
        "CREATE TABLE IF NOT EXISTS Friends ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "friend_id INTEGER NOT NULL,"
        "service_id INTEGER NOT NULL,"
        "remark TEXT,"
        "add_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "UNIQUE(user_id, friend_id, service_id),"
        "FOREIGN KEY(user_id) REFERENCES Users(id),"
        "FOREIGN KEY(friend_id) REFERENCES Users(id),"
        "FOREIGN KEY(service_id) REFERENCES Services(id)"
        ");";

    char* errMsg = nullptr;
    
    // 执行创建Friends表
    int rc = sqlite3_exec(db_, sqlFriends, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "SQL error creating Friends table: " + std::string(errMsg);
        Logger::getInstance().logError(errorMsg);
        sqlite3_free(errMsg);
        return false;
    }
    
    Logger::getInstance().logInfo("Social tables created successfully");
    return true;
}

bool DatabaseManager::activateUserService(int userId, int serviceId) {
    if (!db_) return false;
    
    const char* sql = 
        "INSERT OR IGNORE INTO UserServices (user_id, service_id) "
        "VALUES (?, ?);";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, serviceId);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        Logger::getInstance().logError("Failed to activate service: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    return true;
}

bool DatabaseManager::deactivateUserService(int userId, int serviceId) {
    if (!db_) return false;
    
    const char* sql = 
        "DELETE FROM UserServices WHERE user_id = ? AND service_id = ?;";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, serviceId);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        Logger::getInstance().logError("Failed to deactivate service: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    return true;
}

bool DatabaseManager::queryUserServices(int userId, std::string& jsonResult) {
    if (!db_) return false;
    
    const char* sql = 
        "SELECT s.id, s.name, "
        "CASE WHEN us.id IS NOT NULL THEN 1 ELSE 0 END as activated "
        "FROM Services s "
        "LEFT JOIN UserServices us ON s.id = us.service_id AND us.user_id = ?;";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    
    jsonResult = "[";
    bool first = true;
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int activated = sqlite3_column_int(stmt, 2);
        
        if (!first) {
            jsonResult += ",";
        }
        
        jsonResult += "{\"id\":" + std::to_string(id) + 
                      ",\"name\":\"" + std::string(name) + "\"" +
                      ",\"activated\":" + std::to_string(activated) + "}";
        
        first = false;
    }
    
    jsonResult += "]";
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        Logger::getInstance().logError("Failed to query user services: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    return true;
}

// 根据用户名获取用户ID
int DatabaseManager::getUserIdByUsername(const std::string& username) {
    const char* sql = "SELECT id FROM users WHERE username = ?";
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare statement for getUserIdByUsername");
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    int userId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        userId = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return userId;
}

bool DatabaseManager::addFriend(int userId, int friendId, int serviceId, const std::string& remark) {
    if (!db_) return false;

    // 检查用户是否存在
    const char* checkUserSql = "SELECT 1 FROM Users WHERE id = ?";
    sqlite3_stmt* checkStmt = nullptr;
    
    if (sqlite3_prepare_v2(db_, checkUserSql, -1, &checkStmt, nullptr) != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare user check statement");
        return false;
    }
    
    sqlite3_bind_int(checkStmt, 1, userId);
    bool userExists = (sqlite3_step(checkStmt) == SQLITE_ROW);
    sqlite3_finalize(checkStmt);
    
    if (!userExists) {
        Logger::getInstance().logError("User does not exist: " + std::to_string(userId));
        return false;
    }
    
    // 检查好友用户是否存在
    if (sqlite3_prepare_v2(db_, checkUserSql, -1, &checkStmt, nullptr) != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare friend check statement");
        return false;
    }
    
    sqlite3_bind_int(checkStmt, 1, friendId);
    bool friendExists = (sqlite3_step(checkStmt) == SQLITE_ROW);
    sqlite3_finalize(checkStmt);
    
    if (!friendExists) {
        Logger::getInstance().logError("Friend user does not exist: " + std::to_string(friendId));
        return false;
    }
    
    // 检查是否已经是好友
    const char* checkFriendSql = "SELECT 1 FROM Friends WHERE user_id = ? AND friend_id = ? AND service_id = ?";
    if (sqlite3_prepare_v2(db_, checkFriendSql, -1, &checkStmt, nullptr) != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare friend relationship check statement");
        return false;
    }
    
    sqlite3_bind_int(checkStmt, 1, userId);
    sqlite3_bind_int(checkStmt, 2, friendId);
    sqlite3_bind_int(checkStmt, 3, serviceId);
    bool alreadyFriends = (sqlite3_step(checkStmt) == SQLITE_ROW);
    sqlite3_finalize(checkStmt);
    
    if (alreadyFriends) {
        Logger::getInstance().logInfo("Users are already friends");
        return true; // 已经是好友，返回成功
    }
    
    // 插入好友关系
    const char* insertSql = 
        "INSERT INTO Friends (user_id, friend_id, service_id, remark) VALUES (?, ?, ?, ?);";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, insertSql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare add friend statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, friendId);
    sqlite3_bind_int(stmt, 3, serviceId);
    sqlite3_bind_text(stmt, 4, remark.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        Logger::getInstance().logError("Failed to add friend: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    Logger::getInstance().logInfo("Friend added successfully: user=" + std::to_string(userId) + 
                                 ", friend=" + std::to_string(friendId) + 
                                 ", service=" + std::to_string(serviceId));
    return true;
}

bool DatabaseManager::deleteFriend(int userId, int friendId, int serviceId) {
    if (!db_) return false;

    const char* sql = 
        "DELETE FROM Friends WHERE user_id = ? AND friend_id = ? AND service_id = ?;";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare delete friend statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, friendId);
    sqlite3_bind_int(stmt, 3, serviceId);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        Logger::getInstance().logError("Failed to delete friend: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    Logger::getInstance().logInfo("Friend deleted successfully: user=" + std::to_string(userId) + 
                                 ", friend=" + std::to_string(friendId) + 
                                 ", service=" + std::to_string(serviceId));
    return true;
}

bool DatabaseManager::queryFriends(int userId, int serviceId, std::string& jsonResult) {
    if (!db_) return false;
    
    const char* sql = 
        "SELECT u.id, u.nickname, u.username, f.remark, f.add_time "
        "FROM Friends f "
        "JOIN Users u ON f.friend_id = u.id "
        "WHERE f.user_id = ? AND f.service_id = ? "
        "ORDER BY f.add_time DESC;";

    // 添加日志
    Logger::getInstance().logInfo("Executing SQL: " + std::string(sql));
    Logger::getInstance().logInfo("Parameters - user_id: " + std::to_string(userId) + 
                                 ", service_id: " + std::to_string(serviceId));

    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare query friends statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, serviceId);
    
    jsonResult = "[";
    bool first = true;
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int friendId = sqlite3_column_int(stmt, 0);
        const char* nickname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* remark = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* addTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        
        if (!first) {
            jsonResult += ",";
        }
        
        jsonResult += "{";
        jsonResult += "\"friend_id\":" + std::to_string(friendId) + ",";
        jsonResult += "\"nickname\":\"" + (nickname ? std::string(nickname) : "") + "\",";
        jsonResult += "\"username\":\"" + (username ? std::string(username) : "") + "\",";
        jsonResult += "\"remark\":\"" + (remark ? std::string(remark) : "") + "\",";
        jsonResult += "\"add_time\":\"" + (addTime ? std::string(addTime) : "") + "\"";
        jsonResult += "}";
        
        first = false;
    }
    
    jsonResult += "]";
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        Logger::getInstance().logError("Failed to query friends: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    Logger::getInstance().logInfo("Friends queried successfully: user=" + std::to_string(userId) + 
                                 ", service=" + std::to_string(serviceId));
    return true;
}

bool DatabaseManager::searchUsers(const std::string& keyword, std::string& jsonResult) {
    if (!db_) return false;
    
    const char* sql = 
        "SELECT id, username, nickname, email FROM Users "
        "WHERE username LIKE ? OR nickname LIKE ? OR email LIKE ? "
        "LIMIT 50;";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare search users statement");
        return false;
    }
    
    std::string searchPattern = "%" + keyword + "%";
    sqlite3_bind_text(stmt, 1, searchPattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, searchPattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, searchPattern.c_str(), -1, SQLITE_STATIC);
    
    jsonResult = "[";
    bool first = true;
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (!first) {
            jsonResult += ",";
        }
        
        jsonResult += "{";
        jsonResult += "\"user_id\":" + std::to_string(sqlite3_column_int(stmt, 0)) + ",";
        
        const unsigned char* username = sqlite3_column_text(stmt, 1);
        jsonResult += "\"username\":\"" + (username ? std::string(reinterpret_cast<const char*>(username)) : "") + "\",";
        
        const unsigned char* nickname = sqlite3_column_text(stmt, 2);
        jsonResult += "\"nickname\":\"" + (nickname ? std::string(reinterpret_cast<const char*>(nickname)) : "") + "\",";
        
        const unsigned char* email = sqlite3_column_text(stmt, 3);
        jsonResult += "\"email\":\"" + (email ? std::string(reinterpret_cast<const char*>(email)) : "") + "\"";
        
        jsonResult += "}";
        first = false;
    }
    
    jsonResult += "]";
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

// 创建消息相关表
bool DatabaseManager::createMessageTables() {
    if (!db_) return false;

    // 创建消息表 - 按照设计文档4.2.7节
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS Messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "sender_id INTEGER NOT NULL,"
        "receiver_id INTEGER NOT NULL,"
        "type INTEGER NOT NULL,"
        "content TEXT NOT NULL,"
        "send_time TEXT DEFAULT CURRENT_TIMESTAMP,"
        "service_id INTEGER NOT NULL,"
        "status INTEGER DEFAULT 0,"
        "FOREIGN KEY (sender_id) REFERENCES Users(id),"
        "FOREIGN KEY (receiver_id) REFERENCES Users(id),"
        "FOREIGN KEY (service_id) REFERENCES Services(id)"
        ");";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::string errorMsg = "SQL error: " + std::string(errMsg);
        Logger::getInstance().logError(errorMsg);
        sqlite3_free(errMsg);
        return false;
    }
    
    Logger::getInstance().logInfo("Messages table created successfully");
    return true;
}

// 发送消息
bool DatabaseManager::sendMessage(int senderId, int receiverId, int type, const std::string& content, int serviceId) {
    if (!db_) return false;
    
    // 验证发送者和接收者是否存在
    std::string checkUserSql = "SELECT id FROM Users WHERE id = ?;";
    sqlite3_stmt* checkStmt = nullptr;
    
    // 检查发送者是否存在
    int rc = sqlite3_prepare_v2(db_, checkUserSql.c_str(), -1, &checkStmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(checkStmt, 1, senderId);
    rc = sqlite3_step(checkStmt);
    if (rc != SQLITE_ROW) {
        Logger::getInstance().logError("Sender not found: " + std::to_string(senderId));
        sqlite3_finalize(checkStmt);
        return false;
    }
    sqlite3_reset(checkStmt);
    
    // 检查接收者是否存在
    sqlite3_bind_int(checkStmt, 1, receiverId);
    rc = sqlite3_step(checkStmt);
    if (rc != SQLITE_ROW) {
        Logger::getInstance().logError("Receiver not found: " + std::to_string(receiverId));
        sqlite3_finalize(checkStmt);
        return false;
    }
    sqlite3_finalize(checkStmt);
    
    // 插入消息记录
    std::string sql = "INSERT INTO Messages (sender_id, receiver_id, type, content, service_id) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    
    rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, senderId);
    sqlite3_bind_int(stmt, 2, receiverId);
    sqlite3_bind_int(stmt, 3, type);
    sqlite3_bind_text(stmt, 4, content.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, serviceId);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        Logger::getInstance().logError("Failed to send message: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    Logger::getInstance().logInfo("Message sent successfully from user " + std::to_string(senderId) + 
                                 " to " + std::to_string(receiverId) + 
                                 " for service " + std::to_string(serviceId));
    return true;
}

// 获取消息历史
bool DatabaseManager::getMessageHistory(int userId, int targetId, int type, int serviceId, int page, int pageSize, std::string& jsonResult) {
    if (!db_) return false;
    
    // 计算偏移量
    int offset = (page - 1) * pageSize;
    
    // 构建查询SQL - 查询用户与目标用户之间的对话历史
    std::string sql = 
        "SELECT m.id, m.sender_id, m.receiver_id, m.type, m.content, m.send_time, m.status, "
        "s.nickname as sender_nickname, r.nickname as receiver_nickname "
        "FROM Messages m "
        "JOIN Users s ON m.sender_id = s.id "
        "JOIN Users r ON m.receiver_id = r.id "
        "WHERE m.service_id = ? AND m.type = ? AND "
        "((m.sender_id = ? AND m.receiver_id = ?) OR (m.sender_id = ? AND m.receiver_id = ?)) "
        "ORDER BY m.send_time DESC "
        "LIMIT ? OFFSET ?;";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, serviceId);
    sqlite3_bind_int(stmt, 2, type);
    sqlite3_bind_int(stmt, 3, userId);
    sqlite3_bind_int(stmt, 4, targetId);
    sqlite3_bind_int(stmt, 5, targetId);
    sqlite3_bind_int(stmt, 6, userId);
    sqlite3_bind_int(stmt, 7, pageSize);
    sqlite3_bind_int(stmt, 8, offset);
    
    // 构建JSON结果
    jsonResult = "{\"messages\":[";
    bool first = true;
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (!first) {
            jsonResult += ",";
        }
        
        jsonResult += "{";
        jsonResult += "\"id\":" + std::to_string(sqlite3_column_int(stmt, 0)) + ",";
        jsonResult += "\"sender_id\":" + std::to_string(sqlite3_column_int(stmt, 1)) + ",";
        jsonResult += "\"receiver_id\":" + std::to_string(sqlite3_column_int(stmt, 2)) + ",";
        jsonResult += "\"type\":" + std::to_string(sqlite3_column_int(stmt, 3)) + ",";
        
        const unsigned char* content_raw = sqlite3_column_text(stmt, 4);
        // 【关键修改】: 对从数据库取出的 content 字符串进行转义
        std::string content_str = content_raw ? reinterpret_cast<const char*>(content_raw) : "";
        std::string escaped_content = escapeJsonString(content_str);
        jsonResult += "\"content\":\"" + escaped_content + "\",";
        
        const unsigned char* sendTime = sqlite3_column_text(stmt, 5);
        jsonResult += "\"send_time\":\"" + (sendTime ? std::string(reinterpret_cast<const char*>(sendTime)) : "") + "\",";
        
        jsonResult += "\"status\":" + std::to_string(sqlite3_column_int(stmt, 6)) + ",";
        
        const unsigned char* senderNickname = sqlite3_column_text(stmt, 7);
        jsonResult += "\"sender_nickname\":\"" + (senderNickname ? std::string(reinterpret_cast<const char*>(senderNickname)) : "") + "\",";
        
        const unsigned char* receiverNickname = sqlite3_column_text(stmt, 8);
        jsonResult += "\"receiver_nickname\":\"" + (receiverNickname ? std::string(reinterpret_cast<const char*>(receiverNickname)) : "") + "\"";
        
        jsonResult += "}";
        first = false;
    }
    
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        Logger::getInstance().logError("Failed to query message history: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    // 查询总消息数
    std::string countSql = 
        "SELECT COUNT(*) FROM Messages "
        "WHERE service_id = ? AND type = ? AND "
        "((sender_id = ? AND receiver_id = ?) OR (sender_id = ? AND receiver_id = ?));";
    
    sqlite3_stmt* countStmt = nullptr;
    rc = sqlite3_prepare_v2(db_, countSql.c_str(), -1, &countStmt, nullptr);
    
    int totalCount = 0;
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(countStmt, 1, serviceId);
        sqlite3_bind_int(countStmt, 2, type);
        sqlite3_bind_int(countStmt, 3, userId);
        sqlite3_bind_int(countStmt, 4, targetId);
        sqlite3_bind_int(countStmt, 5, targetId);
        sqlite3_bind_int(countStmt, 6, userId);
        
        if (sqlite3_step(countStmt) == SQLITE_ROW) {
            totalCount = sqlite3_column_int(countStmt, 0);
        }
    }
    sqlite3_finalize(countStmt);
    
    // 完成JSON结果
    jsonResult += "],";
    jsonResult += "\"total\":" + std::to_string(totalCount) + ",";
    jsonResult += "\"page\":" + std::to_string(page) + ",";
    jsonResult += "\"page_size\":" + std::to_string(pageSize);
    jsonResult += "}";
    
    Logger::getInstance().logInfo("Message history queried successfully for user " + std::to_string(userId) + 
                                 " and target " + std::to_string(targetId) + 
                                 " for service " + std::to_string(serviceId));
    return true;
}

// 创建群组相关表
bool DatabaseManager::createGroupTables() {
    // 创建群组表
    const char* createGroupsTableSQL = 
        "CREATE TABLE IF NOT EXISTS Groups ("
        "group_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "group_name TEXT NOT NULL,"
        "creator_id INTEGER NOT NULL,"
        "service_id INTEGER NOT NULL,"
        "description TEXT,"
        "create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY (creator_id) REFERENCES Users(user_id),"
        "FOREIGN KEY (service_id) REFERENCES Services(service_id)"
        ");";
    
    // 创建群成员表
    const char* createGroupMembersTableSQL = 
        "CREATE TABLE IF NOT EXISTS GroupMembers ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "group_id INTEGER NOT NULL,"
        "user_id INTEGER NOT NULL,"
        "service_id INTEGER NOT NULL,"
        "join_type INTEGER NOT NULL,"  // 0: 申请加入(QQ群), 1: 推荐加入(微信群)
        "join_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY (group_id) REFERENCES Groups(group_id),"
        "FOREIGN KEY (user_id) REFERENCES Users(user_id),"
        "FOREIGN KEY (service_id) REFERENCES Services(service_id),"
        "UNIQUE(group_id, user_id, service_id)"
        ");";
    
    char* errMsg = nullptr;
    
    // 执行创建群组表的SQL
    if (sqlite3_exec(db_, createGroupsTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string error = "Failed to create Groups table: ";
        error += errMsg;
        sqlite3_free(errMsg);
        Logger::getInstance().logError(error);
        return false;
    }
    
    // 执行创建群成员表的SQL
    if (sqlite3_exec(db_, createGroupMembersTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string error = "Failed to create GroupMembers table: ";
        error += errMsg;
        sqlite3_free(errMsg);
        Logger::getInstance().logError(error);
        return false;
    }
    
    Logger::getInstance().logInfo("Group tables created successfully");
    return true;
}

// 创建新群组
bool DatabaseManager::createGroup(int creatorId, const std::string& groupName, int serviceId, const std::string& description) {
    const char* sql = "INSERT INTO Groups (group_name, creator_id, service_id, description) VALUES (?, ?, ?, ?);";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare create group statement");
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, groupName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, creatorId);
    sqlite3_bind_int(stmt, 3, serviceId);
    sqlite3_bind_text(stmt, 4, description.c_str(), -1, SQLITE_STATIC);
    
    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        // 获取新创建的群组ID
        int groupId = sqlite3_last_insert_rowid(db_);
        
        // 创建者自动成为群成员
        success = joinGroup(creatorId, groupId, serviceId, 1); // 创建者以推荐方式加入
        
        if (success) {
            Logger::getInstance().logInfo("Group created successfully: " + groupName + 
                                         " by user " + std::to_string(creatorId) + 
                                         " for service " + std::to_string(serviceId));
        } else {
            Logger::getInstance().logError("Failed to add creator to the group");
        }
    } else {
        Logger::getInstance().logError("Failed to create group: " + groupName);
    }
    
    sqlite3_finalize(stmt);
    return success;
}

// 加入群组
bool DatabaseManager::joinGroup(int userId, int groupId, int serviceId, int joinType) {
    // 检查用户是否已在群组中
    if (isUserInGroup(userId, groupId, serviceId)) {
        Logger::getInstance().logError("User " + std::to_string(userId) + 
                                        " is already in group " + std::to_string(groupId));
        return false;
    }
    
    const char* sql = "INSERT INTO GroupMembers (group_id, user_id, service_id, join_type) VALUES (?, ?, ?, ?);";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare join group statement");
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, groupId);
    sqlite3_bind_int(stmt, 2, userId);
    sqlite3_bind_int(stmt, 3, serviceId);
    sqlite3_bind_int(stmt, 4, joinType);
    
    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        success = true;
        Logger::getInstance().logInfo("User " + std::to_string(userId) + 
                                     " joined group " + std::to_string(groupId) + 
                                     " for service " + std::to_string(serviceId) + 
                                     " with join type " + std::to_string(joinType));
    } else {
        Logger::getInstance().logError("Failed to join group: " + std::to_string(groupId) + 
                                      " for user " + std::to_string(userId));
    }
    
    sqlite3_finalize(stmt);
    return success;
}

// 退出群组
bool DatabaseManager::quitGroup(int userId, int groupId, int serviceId) {
    // 检查用户是否在群组中
    if (!isUserInGroup(userId, groupId, serviceId)) {
        Logger::getInstance().logError("User " + std::to_string(userId) + 
                                        " is not in group " + std::to_string(groupId));
        return false;
    }
    
    const char* sql = "DELETE FROM GroupMembers WHERE user_id = ? AND group_id = ? AND service_id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare quit group statement");
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, groupId);
    sqlite3_bind_int(stmt, 3, serviceId);
    
    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        success = true;
        Logger::getInstance().logInfo("User " + std::to_string(userId) + 
                                     " quit group " + std::to_string(groupId) + 
                                     " for service " + std::to_string(serviceId));
    } else {
        Logger::getInstance().logError("Failed to quit group: " + std::to_string(groupId) + 
                                      " for user " + std::to_string(userId));
    }
    
    sqlite3_finalize(stmt);
    return success;
}

// 查询群组成员 - 修改为与其他查询方法一致的风格
bool DatabaseManager::queryGroupMembers(int groupId, int serviceId, std::string& jsonResult) {
    if (!db_) return false;
    
    const char* sql = 
        "SELECT u.id, u.username, u.nickname, gm.join_time "
        "FROM GroupMembers gm "
        "JOIN Users u ON gm.user_id = u.id "
        "WHERE gm.group_id = ? AND gm.service_id = ? "
        "ORDER BY gm.join_time;";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare query group members statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, groupId);
    sqlite3_bind_int(stmt, 2, serviceId);
    
    jsonResult = "[";
    bool first = true;
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int userId = sqlite3_column_int(stmt, 0);
        const char* username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* nickname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* joinTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        
        if (!first) {
            jsonResult += ",";
        }
        
        jsonResult += "{";
        jsonResult += "\"user_id\":" + std::to_string(userId) + ",";
        jsonResult += "\"username\":\"" + (username ? std::string(username) : "") + "\",";
        jsonResult += "\"nickname\":\"" + (nickname ? std::string(nickname) : "") + "\",";
        jsonResult += "\"join_time\":\"" + (joinTime ? std::string(joinTime) : "") + "\"";
        jsonResult += "}";
        
        first = false;
    }
    
    jsonResult += "]";
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        Logger::getInstance().logError("Failed to query group members: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    Logger::getInstance().logInfo("Group members queried successfully: group=" + std::to_string(groupId) + 
                                 ", service=" + std::to_string(serviceId));
    return true;
}

// 检查用户是否在群组中
bool DatabaseManager::isUserInGroup(int userId, int groupId, int serviceId) {
    const char* sql = "SELECT COUNT(*) FROM GroupMembers WHERE user_id = ? AND group_id = ? AND service_id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare check user in group statement");
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, groupId);
    sqlite3_bind_int(stmt, 3, serviceId);
    
    bool isInGroup = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        isInGroup = (sqlite3_column_int(stmt, 0) > 0);
    }
    
    sqlite3_finalize(stmt);
    return isInGroup;
}

bool DatabaseManager::getUserGroups(int userId, int serviceId, std::string& jsonResult) {
    if (!db_) return false;
    
    const char* sql = 
        "SELECT g.group_id, g.group_name, g.description, g.creator_id, g.create_time, "
        "(SELECT COUNT(*) FROM GroupMembers gm WHERE gm.group_id = g.group_id AND gm.service_id = g.service_id) as member_count "
        "FROM Groups g "
        "JOIN GroupMembers gm ON g.group_id = gm.group_id AND g.service_id = gm.service_id "
        "WHERE gm.user_id = ? AND g.service_id = ? "
        "ORDER BY g.create_time DESC;";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger::getInstance().logError("Failed to prepare get user groups statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, serviceId);
    
    jsonResult = "[";
    bool first = true;
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int groupId = sqlite3_column_int(stmt, 0);
        const char* groupName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int creatorId = sqlite3_column_int(stmt, 3);
        const char* createTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        int memberCount = sqlite3_column_int(stmt, 5);
        
        if (!first) {
            jsonResult += ",";
        }
        
        jsonResult += "{";
        jsonResult += "\"group_id\":" + std::to_string(groupId) + ",";
        jsonResult += "\"group_name\":\"" + (groupName ? escapeJsonString(std::string(groupName)) : "") + "\",";
        jsonResult += "\"description\":\"" + (description ? escapeJsonString(std::string(description)) : "") + "\",";
        jsonResult += "\"creator_id\":" + std::to_string(creatorId) + ",";
        jsonResult += "\"create_time\":\"" + (createTime ? std::string(createTime) : "") + "\",";
        jsonResult += "\"member_count\":" + std::to_string(memberCount);
        jsonResult += "}";
        
        first = false;
    }
    
    jsonResult += "]";
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        Logger::getInstance().logError("Failed to get user groups: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    Logger::getInstance().logInfo("User groups queried successfully: user=" + std::to_string(userId) + 
                                 ", service=" + std::to_string(serviceId));
    return true;
}

} // namespace utils
} // namespace iuim
