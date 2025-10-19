#include "../../include/services/friend_controller.h"
#include "../../include/services/friend_handler.h"
#include "../../include/utils/logger.h"

namespace iuim {
namespace services {

void FriendController::handleFriendAdd(const httplib::Request& req, httplib::Response& res) {
    iuim::utils::Logger::getInstance().logInfo("FriendController::handleFriendAdd - OOP wrapper called");
    // 调用原有的处理函数
    iuim::services::handleFriendAdd(req, res);
}

void FriendController::handleFriendDelete(const httplib::Request& req, httplib::Response& res) {
    iuim::utils::Logger::getInstance().logInfo("FriendController::handleFriendDelete - OOP wrapper called");
    // 调用原有的处理函数
    iuim::services::handleFriendDelete(req, res);
}

void FriendController::handleFriendQuery(const httplib::Request& req, httplib::Response& res) {
    iuim::utils::Logger::getInstance().logInfo("FriendController::handleFriendQuery - OOP wrapper called");
    // 调用原有的处理函数
    iuim::services::handleFriendQuery(req, res);
}

void FriendController::handleUserSearch(const httplib::Request& req, httplib::Response& res) {
    iuim::utils::Logger::getInstance().logInfo("FriendController::handleUserSearch - OOP wrapper called");
    // 调用原有的处理函数
    iuim::services::handleUserSearch(req, res);
}

} // namespace services
} // namespace iuim