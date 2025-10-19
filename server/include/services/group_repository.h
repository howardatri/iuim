#ifndef IUIM_GROUP_REPOSITORY_H
#define IUIM_GROUP_REPOSITORY_H

#include <string>
#include <vector>
#include <memory>
#include "../models/qqgroupZHY.h"
#include "../models/wechatgroupZHY.h"
#include "../models/weibogroupZHY.h"

namespace iuim {
namespace services {

// 群组数据访问接口
class IGroupRepository {
public:
    virtual ~IGroupRepository() = default;
    
    // 群组基本操作
    virtual bool createGroup(int creatorId, const std::string& groupName, int serviceId, const std::string& description) = 0;
    virtual bool joinGroup(int userId, int groupId, int serviceId, int joinType) = 0;
    virtual bool quitGroup(int userId, int groupId, int serviceId) = 0;
    virtual bool isUserInGroup(int userId, int groupId, int serviceId) = 0;
    
    // 群组查询操作
    virtual bool queryGroupMembers(int groupId, int serviceId, std::string& jsonResult) = 0;
    virtual bool getUserGroups(int userId, int serviceId, std::string& jsonResult) = 0;
    virtual std::shared_ptr<iuim::models::BaseGroup> getGroupById(int groupId, int serviceId) = 0;
    
    // 群组成员管理
    virtual std::vector<int> getGroupMemberIds(int groupId, int serviceId) = 0;
    virtual int getGroupMemberCount(int groupId, int serviceId) = 0;
};

// 群组数据访问实现类
class GroupRepository : public IGroupRepository {
public:
    GroupRepository();
    virtual ~GroupRepository() = default;
    
    // 实现接口方法
    bool createGroup(int creatorId, const std::string& groupName, int serviceId, const std::string& description) override;
    bool joinGroup(int userId, int groupId, int serviceId, int joinType) override;
    bool quitGroup(int userId, int groupId, int serviceId) override;
    bool isUserInGroup(int userId, int groupId, int serviceId) override;
    
    bool queryGroupMembers(int groupId, int serviceId, std::string& jsonResult) override;
    bool getUserGroups(int userId, int serviceId, std::string& jsonResult) override;
    std::shared_ptr<iuim::models::BaseGroup> getGroupById(int groupId, int serviceId) override;
    
    std::vector<int> getGroupMemberIds(int groupId, int serviceId) override;
    int getGroupMemberCount(int groupId, int serviceId) override;

private:
    // 辅助方法
    std::shared_ptr<iuim::models::BaseGroup> createGroupModel(int id, const std::string& name, 
                                                              const std::string& description, int serviceId);
};

} // namespace services
} // namespace iuim

#endif // IUIM_GROUP_REPOSITORY_H