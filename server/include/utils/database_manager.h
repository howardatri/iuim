#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <memory>
#include "../../../third_party/sqlite/sqlite3.h"

namespace iuim {
namespace utils {

class DatabaseManager {
public:
    // 获取单例实例
    static DatabaseManager& getInstance();
    
    // 禁用拷贝构造和赋值操作
    DatabaseManager(const DatabaseManager&) = delete;
    void operator=(const DatabaseManager&) = delete;
    
    // 注册新用户
    bool registerUser(const std::string& username, const std::string& password, 
                      const std::string& nickname, const std::string& email);
    
    // 验证用户登录
    bool verifyLogin(const std::string& username, const std::string& password, 
                     std::string& nickname, std::string& email);
    
    // 检查用户名是否已存在
    bool isUserExists(const std::string& username);

private:
    // 私有构造函数和析构函数
    DatabaseManager(const std::string& dbPath = "data/iuim.db");
    ~DatabaseManager();
    
    // 初始化数据库 (现在是私有的)
    bool initialize();
    
    // 创建用户表 (现在是私有的)
    bool createUserTable();
    
    // 计算SHA256哈希
    std::string hashPassword(const std::string& password);
    
    // 数据库连接
    sqlite3* db_;
    std::string dbPath_;
};

} // namespace utils
} // namespace iuim

#endif // DATABASE_MANAGER_H