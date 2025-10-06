#include "../../include/utils/database_manager.h"
#include "../../include/utils/logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <fstream>

namespace iuim {
namespace utils {

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
        "(1, 'QQ', '经典即时通讯服务'), (2, '微信', '现代社交生活方式');";

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

} // namespace utils
} // namespace iuim
