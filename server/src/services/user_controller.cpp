#include "../../include/services/user_controller.h"
#include "../../include/services/user_handler.h"
#include "../../include/utils/logger.h"

namespace iuim {
namespace services {

void UserController::handleRegister(const httplib::Request& req, httplib::Response& res) {
    iuim::utils::Logger::getInstance().logInfo("UserController::handleRegister - OOP wrapper called");
    // 调用原有的处理函数
    iuim::services::handleRegister(req, res);
}

void UserController::handleLogin(const httplib::Request& req, httplib::Response& res) {
    iuim::utils::Logger::getInstance().logInfo("UserController::handleLogin - OOP wrapper called");
    // 调用原有的处理函数
    iuim::services::handleLogin(req, res);
}

} // namespace services
} // namespace iuim