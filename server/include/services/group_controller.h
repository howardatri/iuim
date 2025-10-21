#ifndef IUIM_GROUP_CONTROLLER_H
#define IUIM_GROUP_CONTROLLER_H

#include <memory>
#include "group_service.h"
#include "../../../third_party/httplib/httplib.h"
#include "../../../third_party/nlohmann/json.hpp"

namespace iuim {
namespace services {

// 群组控制器类 - 处理HTTP请求和响应
class GroupController {
public:
    explicit GroupController(std::shared_ptr<IGroupService> groupService);
    virtual ~GroupController() = default;
    
    // HTTP请求处理方法
    void handleCreateGroup(const httplib::Request& req, httplib::Response& res);
    void handleJoinGroup(const httplib::Request& req, httplib::Response& res);
    void handleQuitGroup(const httplib::Request& req, httplib::Response& res);
    void handleGetGroupMembers(const httplib::Request& req, httplib::Response& res);
    void handleGetUserGroups(const httplib::Request& req, httplib::Response& res);
    
    // 静态方法用于与现有handler兼容
    static void handleGroupCreate(const httplib::Request& req, httplib::Response& res);
    static void handleGroupJoin(const httplib::Request& req, httplib::Response& res);
    static void handleGroupQuit(const httplib::Request& req, httplib::Response& res);
    static void handleGroupMembers(const httplib::Request& req, httplib::Response& res);
    static void handleGroupList(const httplib::Request& req, httplib::Response& res);
    
    // 增强群组功能的静态包裹器方法
    static void handleGetGroupSettings(const httplib::Request& req, httplib::Response& res);
    static void handleUpdateGroupSettings(const httplib::Request& req, httplib::Response& res);
    static void handleChangeGroupType(const httplib::Request& req, httplib::Response& res);
    static void handleSetMemberRole(const httplib::Request& req, httplib::Response& res);
    static void handleGetMemberRoles(const httplib::Request& req, httplib::Response& res);
    static void handleApplyJoinGroup(const httplib::Request& req, httplib::Response& res);
    static void handleInviteJoinGroup(const httplib::Request& req, httplib::Response& res);
    static void handleFreeJoinTopic(const httplib::Request& req, httplib::Response& res);
    static void handleGetCurrentUserRole(const httplib::Request& req, httplib::Response& res);

private:
    std::shared_ptr<IGroupService> groupService_;
    
    // 辅助方法
    bool parseRequestJson(const std::string& body, nlohmann::json& json, httplib::Response& res);
    bool validateRequiredFields(const nlohmann::json& json, const std::vector<std::string>& fields, httplib::Response& res);
    void sendJsonResponse(httplib::Response& res, const nlohmann::json& json);
    void sendErrorResponse(httplib::Response& res, int code, const std::string& message);
    
    // 单例实例获取
    static GroupController& getInstance();
    
    // 日志记录
    void logRequest(const std::string& operation, const nlohmann::json& requestData);
    void logResponse(const std::string& operation, const nlohmann::json& responseData);
};

} // namespace services
} // namespace iuim

#endif // IUIM_GROUP_CONTROLLER_H