#ifndef IUIM_GROUP_HANDLER_H
#define IUIM_GROUP_HANDLER_H

#include <string>
#include "../../../third_party/httplib/httplib.h"
#include "../../../third_party/nlohmann/json.hpp"

namespace iuim {
namespace services {

// 处理加入群组请求
void handleGroupJoin(const httplib::Request& req, httplib::Response& res);

// 处理退出群组请求
void handleGroupQuit(const httplib::Request& req, httplib::Response& res);

// 处理查询群成员请求
void handleGroupMembers(const httplib::Request& req, httplib::Response& res);

// 处理创建群组请求
void handleGroupCreate(const httplib::Request& req, httplib::Response& res);

// 处理获取用户群组列表请求
void handleGroupList(const httplib::Request& req, httplib::Response& res);

// 增强群组功能处理函数

// 群组设置管理
void handleGetGroupSettings(const httplib::Request& req, httplib::Response& res);
void handleUpdateGroupSettings(const httplib::Request& req, httplib::Response& res);
void handleChangeGroupType(const httplib::Request& req, httplib::Response& res);

// 成员角色管理
void handleSetMemberRole(const httplib::Request& req, httplib::Response& res);
void handleGetMemberRoles(const httplib::Request& req, httplib::Response& res);
void handleGetCurrentUserRole(const httplib::Request& req, httplib::Response& res);

// 特色功能接口
void handleApplyJoinGroup(const httplib::Request& req, httplib::Response& res);
void handleInviteJoinGroup(const httplib::Request& req, httplib::Response& res);
void handleFreeJoinTopic(const httplib::Request& req, httplib::Response& res);

} // namespace services
} // namespace iuim

#endif // IUIM_GROUP_HANDLER_H