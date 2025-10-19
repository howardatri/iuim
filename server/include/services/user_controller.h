#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include "../../../third_party/httplib/httplib.h"

namespace iuim {
namespace services {

/**
 * UserController - 用户管理控制器
 * 为现有的用户处理函数提供面向对象的包裹器
 */
class UserController {
public:
    // 禁止实例化
    UserController() = delete;
    UserController(const UserController&) = delete;
    UserController& operator=(const UserController&) = delete;
    
    // 静态方法作为包裹器
    static void handleRegister(const httplib::Request& req, httplib::Response& res);
    static void handleLogin(const httplib::Request& req, httplib::Response& res);
};

} // namespace services
} // namespace iuim

#endif // USER_CONTROLLER_H