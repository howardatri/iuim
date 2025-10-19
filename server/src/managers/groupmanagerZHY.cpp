#include "../../include/managers/groupmanagerZHY.h"

namespace iuim {
namespace managers {

bool GroupManagerZHY::createGroup(int creatorId, const std::string& groupName, int serviceId, const std::string& description) {
    return utils::DatabaseManager::getInstance().createGroup(creatorId, groupName, serviceId, description);
}

bool GroupManagerZHY::joinGroup(int userId, int groupId, int serviceId, int role) {
    return utils::DatabaseManager::getInstance().joinGroup(userId, groupId, serviceId, role);
}

bool GroupManagerZHY::quitGroup(int userId, int groupId, int serviceId) {
    return utils::DatabaseManager::getInstance().quitGroup(userId, groupId, serviceId);
}

bool GroupManagerZHY::queryGroupMembers(int groupId, int serviceId, std::string& jsonResult) {
    return utils::DatabaseManager::getInstance().queryGroupMembers(groupId, serviceId, jsonResult);
}

bool GroupManagerZHY::getUserGroups(int userId, int serviceId, std::string& jsonResult) {
    return utils::DatabaseManager::getInstance().getUserGroups(userId, serviceId, jsonResult);
}

} // namespace managers
} // namespace iuim