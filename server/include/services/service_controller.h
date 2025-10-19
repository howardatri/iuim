#ifndef SERVICE_CONTROLLER_H
#define SERVICE_CONTROLLER_H

#include "../../../third_party/httplib/httplib.h"

namespace iuim {
namespace services {

/**
 * ServiceController - 服务管理控制器
 * 为现有的服务处理函数提供面向对象的包裹器
 */
class ServiceController {
public:
    // 禁止实例化
    ServiceController() = delete;
    ServiceController(const ServiceController&) = delete;
    ServiceController& operator=(const ServiceController&) = delete;
    
    // 静态方法作为包裹器
    static void handleActivate(const httplib::Request& req, httplib::Response& res);
    static void handleDeactivate(const httplib::Request& req, httplib::Response& res);
    static void handleQueryUserServices(const httplib::Request& req, httplib::Response& res);
};

} // namespace services
} // namespace iuim

#endif // SERVICE_CONTROLLER_H