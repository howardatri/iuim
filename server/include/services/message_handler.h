#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include "../../../third_party/httplib/httplib.h"
#include <string>

namespace iuim {
namespace services {

/**
 * 处理发送消息请求
 * @param req HTTP请求对象
 * @param res HTTP响应对象
 */
void handleMessageSend(const httplib::Request& req, httplib::Response& res);

/**
 * 处理查询历史消息请求
 * @param req HTTP请求对象
 * @param res HTTP响应对象
 */
void handleMessageHistory(const httplib::Request& req, httplib::Response& res);

} // namespace services
} // namespace iuim

#endif // MESSAGE_HANDLER_H