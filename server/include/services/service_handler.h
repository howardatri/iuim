#ifndef SERVICE_HANDLER_H
#define SERVICE_HANDLER_H

#include <string>
#include "../utils/database_manager.h"
#include "../utils/logger.h"
#include "../../../third_party/httplib/httplib.h"
#include "../../../third_party/nlohmann/json.hpp"

namespace iuim {
namespace services {

// 处理服务激活请求
void handleActivate(const httplib::Request& req, httplib::Response& res);

// 处理服务停用请求
void handleDeactivate(const httplib::Request& req, httplib::Response& res);

// 处理查询用户服务请求
void handleQueryUserServices(const httplib::Request& req, httplib::Response& res);

} // namespace services
} // namespace iuim

#endif // SERVICE_HANDLER_H