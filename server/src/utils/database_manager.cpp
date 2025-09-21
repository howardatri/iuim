#include "../../include/utils/database_manager.h"
#include "../../include/utils/logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <fstream>

namespace iuim {
namespace utils {

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
        iuim::utils::Logger::logError(errorMsg);
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    iuim::utils::Logger::logInfo("Database connection opened successfully: " + dbPath_);
    
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
        iuim::utils::Logger::logError(errorMsg);
        sqlite3_free(errMsg);
        return false;
    }
    
    iuim::utils::Logger::logInfo("User table created or already exists");
    return true;
}

bool DatabaseManager::registerUser(const std::string& username, const std::string& password, 
                                  const std::string& nickname, const std::string& email) {
    if (!db_) {
        iuim::utils::Logger::logError("Database not initialized");
        return false;
    }

    // 检查用户是否已存在
    if (isUserExists(username)) {
        iuim::utils::Logger::logInfo("Registration failed: Username already exists: " + username);
        return false;
    }
    
    iuim::utils::Logger::logInfo("Registering new user: " + username);

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

} // namespace utils
} // namespace iuim