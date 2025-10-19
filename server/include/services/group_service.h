#ifndef IUIM_GROUP_SERVICE_H
#define IUIM_GROUP_SERVICE_H

#include <string>
#include <memory>
#include "group_repository.h"
#include "../../../third_party/nlohmann/json.hpp"

namespace iuim {
namespace services {

// 群组业务服务接口
class IGroupService {
public:
    virtual ~IGroupService() = default;
    
    // 群组业务操作
    virtual nlohmann::json createGroup(int creatorId, const std::string& groupName, int serviceId, const std::string& description) = 0;
    virtual nlohmann::json joinGroup(int userId, int groupId, int serviceId, int joinType) = 0;
    virtual nlohmann::json quitGroup(int userId, int groupId, int serviceId) = 0;
    virtual nlohmann::json getGroupMembers(int groupId, int serviceId) = 0;
    virtual nlohmann::json getUserGroups(int userId, int serviceId) = 0;
    
    // 群组验证和查询
    virtual bool validateGroupAccess(int userId, int groupId, int serviceId) = 0;
    virtual bool validateJoinType(int joinType) = 0;
    virtual std::shared_ptr<iuim::models::BaseGroup> getGroupInfo(int groupId, int serviceId) = 0;
};

// 群组业务服务实现类
class GroupService : public IGroupService {
public:
    explicit GroupService(std::shared_ptr<IGroupRepository> repository);
    virtual ~GroupService() = default;
    
    // 实现业务接口
    nlohmann::json createGroup(int creatorId, const std::string& groupName, int serviceId, const std::string& description) override;
    nlohmann::json joinGroup(int userId, int groupId, int serviceId, int joinType) override;
    nlohmann::json quitGroup(int userId, int groupId, int serviceId) override;
    nlohmann::json getGroupMembers(int groupId, int serviceId) override;
    nlohmann::json getUserGroups(int userId, int serviceId) override;
    
    // 验证和查询方法
    bool validateGroupAccess(int userId, int groupId, int serviceId) override;
    bool validateJoinType(int joinType) override;
    std::shared_ptr<iuim::models::BaseGroup> getGroupInfo(int groupId, int serviceId) override;

private:
    std::shared_ptr<IGroupRepository> repository_;
    
    // 辅助方法
    nlohmann::json createSuccessResponse(const std::string& message, const nlohmann::json& data = nlohmann::json::object());
    nlohmann::json createErrorResponse(int code, const std::string& message);
    bool validateServiceId(int serviceId);
    std::string getServiceName(int serviceId);
};

} // namespace services
} // namespace iuim

#endif // IUIM_GROUP_SERVICE_H