#ifndef IUIM_FRIEND_CONTROLLER_H
#define IUIM_FRIEND_CONTROLLER_H

#include "../../../third_party/httplib/httplib.h"

namespace iuim {
namespace services {

/**
 * @brief 好友控制器 - 简单包裹器模式
 * 提供面向对象的接口来包裹现有的好友处理函数
 */
class FriendController {
public:
    // 静态方法 - 用于与现有handler兼容
    static void handleFriendAdd(const httplib::Request& req, httplib::Response& res);
    static void handleFriendDelete(const httplib::Request& req, httplib::Response& res);
    static void handleFriendQuery(const httplib::Request& req, httplib::Response& res);
    static void handleUserSearch(const httplib::Request& req, httplib::Response& res);
    
    // 新增：跨服务好友功能
    static void handleQueryCommonFriends(const httplib::Request& req, httplib::Response& res);
    static void handleQueryCrossServiceFriends(const httplib::Request& req, httplib::Response& res);

private:
    // 私有构造函数，防止实例化
    FriendController() = default;
};

} // namespace services
} // namespace iuim

#endif // IUIM_FRIEND_CONTROLLER_H