#ifndef USER_HANDLER_H
#define USER_HANDLER_H

#include "../../../third_party/httplib/httplib.h"

namespace iuim {
namespace services {

// 处理用户注册请求
void handleRegister(const httplib::Request& req, httplib::Response& res);

// 处理用户登录请求
void handleLogin(const httplib::Request& req, httplib::Response& res);

} // namespace services
} // namespace iuim

#endif // USER_HANDLER_H