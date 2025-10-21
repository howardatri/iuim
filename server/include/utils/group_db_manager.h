#ifndef GROUP_DB_MANAGER_H
#define GROUP_DB_MANAGER_H

#include <string>
#include <memory>
#include "../../../third_party/sqlite/sqlite3.h"

namespace iuim {
namespace utils {

class GroupDBManager {
public:
    // 获取单例实例
    static GroupDBManager& getInstance();
    
    // 禁用拷贝构造和赋值操作
    GroupDBManager(const GroupDBManager&) = delete;
    void operator=(const GroupDBManager&) = delete;
    
    // 群组设置管理
    bool getGroupSettings(int groupId, int serviceId, std::string& jsonResult);
    bool updateGroupSettings(int groupId, int serviceId, int groupType, int joinMethod, 
                           int allowSubgroups, int adminSystem, int maxMembers);
    bool changeGroupType(int groupId, int sourceServiceId, int targetServiceId);
    
    // 成员角色管理
    bool setMemberRole(int groupId, int userId, int serviceId, int roleType);
    bool getMemberRoles(int groupId, int serviceId, std::string& jsonResult);
    bool getCurrentUserRole(int groupId, int userId, std::string& jsonResult);
    
    // 服务特定加入方式
    bool applyJoinGroup(int groupId, int userId, int serviceId, const std::string& reason);
    bool inviteJoinGroup(int groupId, int inviterId, int inviteeId, int serviceId);
    bool freeJoinTopic(int groupId, int userId, int serviceId);

private:
    // 私有构造函数和析构函数
    GroupDBManager(const std::string& dbPath = "data/iuim.db");
    ~GroupDBManager();
    

    
    // 设置服务默认配置
    void setServiceDefaultSettings(int serviceId, int& groupType, int& joinMethod, 
                                 int& allowSubgroups, int& adminSystem, int& maxMembers);
    
    // 辅助方法：检查群组是否存在
    bool isGroupExists(int groupId);
    
    // 辅助方法：检查用户是否在群组中
    bool isUserInGroup(int userId, int groupId, int serviceId);
    
    // 辅助方法：获取用户信息
    bool getUserInfo(int userId, std::string& username, std::string& nickname);
    
    // 数据库连接
    sqlite3* db_;
    std::string dbPath_;
};

std::string escapeJsonString(const std::string& input);

} // namespace utils
} // namespace iuim

#endif // GROUP_DB_MANAGER_H