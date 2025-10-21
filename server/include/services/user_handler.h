#ifndef USER_HANDLER_H
#define USER_HANDLER_H

#include "../../../third_party/httplib/httplib.h"

namespace iuim {
namespace services {

// 处理用户注册请求
void handleRegister(const httplib::Request& req, httplib::Response& res);

// 处理用户登录请求
void handleLogin(const httplib::Request& req, httplib::Response& res);

// 处理用户资料更新请求
void handleUpdateProfile(const httplib::Request& req, httplib::Response& res);

// 处理微信绑定请求
void handleBindWechat(const httplib::Request& req, httplib::Response& res);

// 处理获取用户资料请求
void handleGetProfile(const httplib::Request& req, httplib::Response& res);

} // namespace services
} // namespace iuim

#endif // USER_HANDLER_H