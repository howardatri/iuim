#include "../../include/services/message_controller.h"
#include "../../include/services/message_handler.h"
#include "../../include/utils/logger.h"

namespace iuim {
namespace services {

void MessageController::handleMessageSend(const httplib::Request& req, httplib::Response& res) {
    iuim::utils::Logger::getInstance().logInfo("MessageController::handleMessageSend - OOP wrapper called");
    // 调用原有的处理函数
    iuim::services::handleMessageSend(req, res);
}

void MessageController::handleMessageHistory(const httplib::Request& req, httplib::Response& res) {
    iuim::utils::Logger::getInstance().logInfo("MessageController::handleMessageHistory - OOP wrapper called");
    // 调用原有的处理函数
    iuim::services::handleMessageHistory(req, res);
}

} // namespace services
} // namespace iuim