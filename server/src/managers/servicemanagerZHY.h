#include "../../include/managers/servicemanagerZHY.h"

namespace iuim {
namespace managers {

bool ServiceManagerZHY::addUserServic(int userId, int serviceId) {
    return utils::DatabaseManager::getInstance().activateUserService(userId, serviceId);
}

bool ServiceManagerZHY::removeUserService(int userId, int serviceId) {
    return utils::DatabaseManager::getInstance().deactivateUserService(userId, serviceId);
}

bool ServiceManagerZHY::queryUserServices(int userId, std::string& jsonResult) {
    return utils::DatabaseManager::getInstance().queryUserServices(userId, jsonResult);
}

} // namespace managers
} // namespace iuim