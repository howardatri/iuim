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
    
    // 获取数据库连接（供其他管理器使用）
    sqlite3* getDatabase() const;
    
    // 注册新用户
    bool registerUser(const std::string& username, const std::string& password, 
                      const std::string& nickname, const std::string& email);
    
    // 验证用户登录
    bool verifyLogin(const std::string& username, const std::string& password, 
                     std::string& nickname, std::string& email);
    
    // 检查用户名是否已存在
    bool isUserExists(const std::string& username);
    
    // 激活用户服务
    bool activateUserService(int userId, int serviceId);
    
    // 停用用户服务
    bool deactivateUserService(int userId, int serviceId);
    
    // 查询用户的所有服务及其状态
    bool queryUserServices(int userId, std::string& jsonResult);
    
    // 根据用户名获取用户ID
    int getUserIdByUsername(const std::string& username);
    
    // 好友管理相关方法
    bool addFriend(int userId, int friendId, int serviceId, const std::string& remark);
    bool deleteFriend(int userId, int friendId, int serviceId);
    bool queryFriends(int userId, int serviceId, std::string& jsonResult);
    bool searchUsers(const std::string& keyword, std::string& jsonResult);
    
    // 消息管理相关方法
    bool sendMessage(int senderId, int receiverId, int type, const std::string& content, int serviceId);
    bool getMessageHistory(int userId, int targetId, int type, int serviceId, int page, int pageSize, std::string& jsonResult);
    
    // 群组管理相关方法
    bool joinGroup(int userId, int groupId, int serviceId, int joinType);
    bool quitGroup(int userId, int groupId, int serviceId);
    bool queryGroupMembers(int groupId, int serviceId, std::string& jsonResult);
    bool createGroup(int creatorId, const std::string& groupName, int serviceId, const std::string& description);
    bool isUserInGroup(int userId, int groupId, int serviceId);
    bool getUserGroups(int userId, int serviceId, std::string& jsonResult);
    
    // 用户资料管理相关方法
    bool updateUserProfile(int user_id, const std::string& nickname, const std::string& birth_date, const std::string& location);
    bool bindWechatId(int user_id, const std::string& wechat_id);
    bool unbindWechatId(int user_id);
    bool setQqId(int user_id, const std::string& qq_id);
    bool getUserFullProfile(int user_id, std::string& jsonResult);
    bool isWechatIdAvailable(const std::string& wechat_id);
    bool isQqIdAvailable(const std::string& qq_id);

private:
    // 私有构造函数和析构函数
    DatabaseManager(const std::string& dbPath = "data/iuim.db");
    ~DatabaseManager();
    
    // 初始化数据库 (现在是私有的)
    bool initialize();
    
    // 创建用户表 (现在是私有的)
    bool createUserTable();
    
    // 创建服务表和用户服务关系表 (私有的)
    bool createServiceTables();
    
    // 创建社交相关表 (私有的)
    bool createSocialTables();
    
    // 创建消息相关表 (私有的)
    bool createMessageTables();
    
    // 创建群组相关表 (私有的)
    bool createGroupTables();
    
    // 计算SHA256哈希
    std::string hashPassword(const std::string& password);
    
    // 数据库连接
    sqlite3* db_;
    std::string dbPath_;
};

} // namespace utils
} // namespace iuim

#endif // DATABASE_MANAGER_H