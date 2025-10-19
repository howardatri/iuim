#include "../../include/services/group_repository.h"
#include "../../include/services/group_factory.h"
#include "../../include/utils/database_manager.h"
#include "../../include/utils/logger.h"

namespace iuim {
namespace services {

GroupRepository::GroupRepository() {
    iuim::utils::Logger::getInstance().logInfo("GroupRepository initialized");
}

bool GroupRepository::createGroup(int creatorId, const std::string& groupName, int serviceId, const std::string& description) {
    try {
        bool result = iuim::utils::DatabaseManager::getInstance().createGroup(creatorId, groupName, serviceId, description);
        if (result) {
            iuim::utils::Logger::getInstance().logInfo("Group created successfully: " + groupName + " for service " + std::to_string(serviceId));
        } else {
            iuim::utils::Logger::getInstance().logError("Failed to create group: " + groupName);
        }
        return result;
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in createGroup: " + std::string(e.what()));
        return false;
    }
}

bool GroupRepository::joinGroup(int userId, int groupId, int serviceId, int joinType) {
    try {
        // 检查用户是否已在群组中
        if (isUserInGroup(userId, groupId, serviceId)) {
            iuim::utils::Logger::getInstance().logInfo("User " + std::to_string(userId) + " is already in group " + std::to_string(groupId));
            return false;
        }
        
        bool result = iuim::utils::DatabaseManager::getInstance().joinGroup(userId, groupId, serviceId, joinType);
        if (result) {
            iuim::utils::Logger::getInstance().logInfo("User " + std::to_string(userId) + " joined group " + std::to_string(groupId));
        } else {
            iuim::utils::Logger::getInstance().logError("Failed to join group for user " + std::to_string(userId));
        }
        return result;
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in joinGroup: " + std::string(e.what()));
        return false;
    }
}

bool GroupRepository::quitGroup(int userId, int groupId, int serviceId) {
    try {
        // 检查用户是否在群组中
        if (!isUserInGroup(userId, groupId, serviceId)) {
            iuim::utils::Logger::getInstance().logInfo("User " + std::to_string(userId) + " is not in group " + std::to_string(groupId));
            return false;
        }
        
        bool result = iuim::utils::DatabaseManager::getInstance().quitGroup(userId, groupId, serviceId);
        if (result) {
            iuim::utils::Logger::getInstance().logInfo("User " + std::to_string(userId) + " quit group " + std::to_string(groupId));
        } else {
            iuim::utils::Logger::getInstance().logError("Failed to quit group for user " + std::to_string(userId));
        }
        return result;
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in quitGroup: " + std::string(e.what()));
        return false;
    }
}

bool GroupRepository::isUserInGroup(int userId, int groupId, int serviceId) {
    try {
        return iuim::utils::DatabaseManager::getInstance().isUserInGroup(userId, groupId, serviceId);
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in isUserInGroup: " + std::string(e.what()));
        return false;
    }
}

bool GroupRepository::queryGroupMembers(int groupId, int serviceId, std::string& jsonResult) {
    try {
        bool result = iuim::utils::DatabaseManager::getInstance().queryGroupMembers(groupId, serviceId, jsonResult);
        if (result) {
            iuim::utils::Logger::getInstance().logInfo("Successfully queried members for group " + std::to_string(groupId));
        } else {
            iuim::utils::Logger::getInstance().logError("Failed to query members for group " + std::to_string(groupId));
        }
        return result;
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in queryGroupMembers: " + std::string(e.what()));
        return false;
    }
}

bool GroupRepository::getUserGroups(int userId, int serviceId, std::string& jsonResult) {
    try {
        bool result = iuim::utils::DatabaseManager::getInstance().getUserGroups(userId, serviceId, jsonResult);
        if (result) {
            iuim::utils::Logger::getInstance().logInfo("Successfully queried groups for user " + std::to_string(userId));
        } else {
            iuim::utils::Logger::getInstance().logError("Failed to query groups for user " + std::to_string(userId));
        }
        return result;
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in getUserGroups: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<iuim::models::BaseGroup> GroupRepository::getGroupById(int groupId, int serviceId) {
    try {
        // 这里需要从数据库查询群组信息
        // 由于现有的DatabaseManager没有直接的getGroupById方法，我们需要通过其他方式获取
        // 暂时返回一个基本的群组对象，实际实现中应该从数据库查询详细信息
        std::string groupName = "Group_" + std::to_string(groupId);
        std::string description = "Group description";
        
        return createGroupModel(groupId, groupName, description, serviceId);
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in getGroupById: " + std::string(e.what()));
        return nullptr;
    }
}

std::vector<int> GroupRepository::getGroupMemberIds(int groupId, int serviceId) {
    std::vector<int> memberIds;
    try {
        std::string jsonResult;
        if (queryGroupMembers(groupId, serviceId, jsonResult)) {
            // 解析JSON结果获取成员ID列表
            // 这里需要解析JSON，提取成员ID
            // 暂时返回空列表，实际实现中应该解析JSON
        }
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in getGroupMemberIds: " + std::string(e.what()));
    }
    return memberIds;
}

int GroupRepository::getGroupMemberCount(int groupId, int serviceId) {
    try {
        std::vector<int> memberIds = getGroupMemberIds(groupId, serviceId);
        return static_cast<int>(memberIds.size());
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in getGroupMemberCount: " + std::string(e.what()));
        return 0;
    }
}

std::shared_ptr<iuim::models::BaseGroup> GroupRepository::createGroupModel(int id, const std::string& name, 
                                                                           const std::string& description, int serviceId) {
    return GroupFactory::createGroup(id, name, description, serviceId);
}

} // namespace services
} // namespace iuim