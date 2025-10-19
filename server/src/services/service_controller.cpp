#include "../../include/services/service_controller.h"
#include "../../include/services/service_handler.h"
#include "../../include/utils/logger.h"

namespace iuim {
namespace services {

void ServiceController::handleActivate(const httplib::Request& req, httplib::Response& res) {
    iuim::utils::Logger::getInstance().logInfo("ServiceController::handleActivate - OOP wrapper called");
    // 调用原有的处理函数
    iuim::services::handleActivate(req, res);
}

void ServiceController::handleDeactivate(const httplib::Request& req, httplib::Response& res) {
    iuim::utils::Logger::getInstance().logInfo("ServiceController::handleDeactivate - OOP wrapper called");
    // 调用原有的处理函数
    iuim::services::handleDeactivate(req, res);
}

void ServiceController::handleQueryUserServices(const httplib::Request& req, httplib::Response& res) {
    iuim::utils::Logger::getInstance().logInfo("ServiceController::handleQueryUserServices - OOP wrapper called");
    // 调用原有的处理函数
    iuim::services::handleQueryUserServices(req, res);
}

} // namespace services
} // namespace iuim