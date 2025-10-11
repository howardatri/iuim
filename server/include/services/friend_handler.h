#ifndef IUIM_FRIEND_HANDLER_H
#define IUIM_FRIEND_HANDLER_H

#include <string>
#include "../../../third_party/httplib/httplib.h"

namespace iuim {
namespace services {

/**
 * @brief 处理好友添加请求
 * 
 * @param req HTTP请求
 * @param res HTTP响应
 */
void handleFriendAdd(const httplib::Request& req, httplib::Response& res);

/**
 * @brief 处理好友删除请求
 * 
 * @param req HTTP请求
 * @param res HTTP响应
 */
void handleFriendDelete(const httplib::Request& req, httplib::Response& res);

/**
 * @brief 处理好友查询请求
 * 
 * @param req HTTP请求
 * @param res HTTP响应
 */
void handleFriendQuery(const httplib::Request& req, httplib::Response& res);

void handleUserSearch(const httplib::Request& req, httplib::Response& res);
} // namespace services
} // namespace iuim

#endif // IUIM_FRIEND_HANDLER_H