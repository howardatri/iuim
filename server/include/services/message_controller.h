#ifndef IUIM_MESSAGE_CONTROLLER_H
#define IUIM_MESSAGE_CONTROLLER_H

#include "../../../third_party/httplib/httplib.h"

namespace iuim {
namespace services {

/**
 * @brief 消息控制器 - 简单包裹器模式
 * 提供面向对象的接口来包裹现有的消息处理函数
 */
class MessageController {
public:
    // 静态方法 - 用于与现有handler兼容
    static void handleMessageSend(const httplib::Request& req, httplib::Response& res);
    static void handleMessageHistory(const httplib::Request& req, httplib::Response& res);

private:
    // 私有构造函数，防止实例化
    MessageController() = default;
};

} // namespace services
} // namespace iuim

#endif // IUIM_MESSAGE_CONTROLLER_H