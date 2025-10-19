#include "../../include/services/group_service.h"
#include "../../include/services/group_factory.h"
#include "../../include/utils/logger.h"

namespace iuim {
namespace services {

GroupService::GroupService(std::shared_ptr<IGroupRepository> repository) 
    : repository_(repository) {
    if (!repository_) {
        throw std::invalid_argument("Repository cannot be null");
    }
    iuim::utils::Logger::getInstance().logInfo("GroupService initialized");
}

nlohmann::json GroupService::createGroup(int creatorId, const std::string& groupName, int serviceId, const std::string& description) {
    try {
        // 验证输入参数
        if (groupName.empty()) {
            return createErrorResponse(400, "Group name cannot be empty");
        }
        
        if (!validateServiceId(serviceId)) {
            return createErrorResponse(400, "Invalid service ID: " + std::to_string(serviceId));
        }
        
        // 调用仓储层创建群组
        bool success = repository_->createGroup(creatorId, groupName, serviceId, description);
        
        if (success) {
            nlohmann::json data;
            data["group_name"] = groupName;
            data["creator_id"] = creatorId;
            data["service_id"] = serviceId;
            data["service_name"] = getServiceName(serviceId);
            
            iuim::utils::Logger::getInstance().logInfo("Group created successfully: " + groupName);
            return createSuccessResponse("Group created successfully", data);
        } else {
            return createErrorResponse(500, "Failed to create group");
        }
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in createGroup: " + std::string(e.what()));
        return createErrorResponse(500, "Internal server error: " + std::string(e.what()));
    }
}

nlohmann::json GroupService::joinGroup(int userId, int groupId, int serviceId, int joinType) {
    try {
        // 验证输入参数
        if (!validateServiceId(serviceId)) {
            return createErrorResponse(400, "Invalid service ID: " + std::to_string(serviceId));
        }
        
        if (!validateJoinType(joinType)) {
            return createErrorResponse(400, "Invalid join type, must be 0 (apply) or 1 (invite)");
        }
        
        // 检查用户是否已在群组中
        if (repository_->isUserInGroup(userId, groupId, serviceId)) {
            return createErrorResponse(409, "User is already in the group");
        }
        
        // 调用仓储层加入群组
        bool success = repository_->joinGroup(userId, groupId, serviceId, joinType);
        
        if (success) {
            nlohmann::json data;
            data["user_id"] = userId;
            data["group_id"] = groupId;
            data["service_id"] = serviceId;
            data["service_name"] = getServiceName(serviceId);
            data["join_type"] = joinType;
            
            iuim::utils::Logger::getInstance().logInfo("User " + std::to_string(userId) + " joined group " + std::to_string(groupId));
            return createSuccessResponse("Successfully joined the group", data);
        } else {
            return createErrorResponse(500, "Failed to join the group");
        }
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in joinGroup: " + std::string(e.what()));
        return createErrorResponse(500, "Internal server error: " + std::string(e.what()));
    }
}

nlohmann::json GroupService::quitGroup(int userId, int groupId, int serviceId) {
    try {
        // 验证输入参数
        if (!validateServiceId(serviceId)) {
            return createErrorResponse(400, "Invalid service ID: " + std::to_string(serviceId));
        }
        
        // 检查用户是否在群组中
        if (!repository_->isUserInGroup(userId, groupId, serviceId)) {
            return createErrorResponse(404, "User is not in the group");
        }
        
        // 调用仓储层退出群组
        bool success = repository_->quitGroup(userId, groupId, serviceId);
        
        if (success) {
            nlohmann::json data;
            data["user_id"] = userId;
            data["group_id"] = groupId;
            data["service_id"] = serviceId;
            data["service_name"] = getServiceName(serviceId);
            
            iuim::utils::Logger::getInstance().logInfo("User " + std::to_string(userId) + " quit group " + std::to_string(groupId));
            return createSuccessResponse("Successfully quit the group", data);
        } else {
            return createErrorResponse(500, "Failed to quit the group");
        }
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in quitGroup: " + std::string(e.what()));
        return createErrorResponse(500, "Internal server error: " + std::string(e.what()));
    }
}

nlohmann::json GroupService::getGroupMembers(int groupId, int serviceId) {
    try {
        // 验证输入参数
        if (!validateServiceId(serviceId)) {
            return createErrorResponse(400, "Invalid service ID: " + std::to_string(serviceId));
        }
        
        // 调用仓储层查询群成员
        std::string jsonResult;
        bool success = repository_->queryGroupMembers(groupId, serviceId, jsonResult);
        
        if (success) {
            // 解析数据库返回的JSON结果
            nlohmann::json membersData = nlohmann::json::parse(jsonResult);
            
            nlohmann::json response;
            response["code"] = 0;
            response["message"] = "Successfully retrieved group members";
            response["data"]["members"] = membersData;
            response["data"]["group_id"] = groupId;
            response["data"]["service_id"] = serviceId;
            response["data"]["service_name"] = getServiceName(serviceId);
            
            return response;
        } else {
            return createErrorResponse(500, "Failed to retrieve group members");
        }
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in getGroupMembers: " + std::string(e.what()));
        return createErrorResponse(500, "Internal server error: " + std::string(e.what()));
    }
}

nlohmann::json GroupService::getUserGroups(int userId, int serviceId) {
    try {
        // 验证输入参数
        if (!validateServiceId(serviceId)) {
            return createErrorResponse(400, "Invalid service ID: " + std::to_string(serviceId));
        }
        
        // 调用仓储层查询用户群组
        std::string jsonResult;
        bool success = repository_->getUserGroups(userId, serviceId, jsonResult);
        
        if (success) {
            // 解析数据库返回的JSON结果
            nlohmann::json groupsData = nlohmann::json::parse(jsonResult);
            
            nlohmann::json response;
            response["code"] = 0;
            response["message"] = "Successfully retrieved user groups";
            response["data"]["groups"] = groupsData;
            response["data"]["user_id"] = userId;
            response["data"]["service_id"] = serviceId;
            response["data"]["service_name"] = getServiceName(serviceId);
            
            return response;
        } else {
            return createErrorResponse(500, "Failed to retrieve user groups");
        }
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in getUserGroups: " + std::string(e.what()));
        return createErrorResponse(500, "Internal server error: " + std::string(e.what()));
    }
}

bool GroupService::validateGroupAccess(int userId, int groupId, int serviceId) {
    try {
        return repository_->isUserInGroup(userId, groupId, serviceId);
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in validateGroupAccess: " + std::string(e.what()));
        return false;
    }
}

bool GroupService::validateJoinType(int joinType) {
    return joinType == 0 || joinType == 1; // 0: apply, 1: invite
}

std::shared_ptr<iuim::models::BaseGroup> GroupService::getGroupInfo(int groupId, int serviceId) {
    try {
        return repository_->getGroupById(groupId, serviceId);
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in getGroupInfo: " + std::string(e.what()));
        return nullptr;
    }
}

nlohmann::json GroupService::createSuccessResponse(const std::string& message, const nlohmann::json& data) {
    nlohmann::json response;
    response["code"] = 0;
    response["message"] = message;
    if (!data.empty()) {
        response["data"] = data;
    }
    return response;
}

nlohmann::json GroupService::createErrorResponse(int code, const std::string& message) {
    nlohmann::json response;
    response["code"] = code;
    response["message"] = message;
    return response;
}

bool GroupService::validateServiceId(int serviceId) {
    return GroupFactory::isValidServiceId(serviceId);
}

std::string GroupService::getServiceName(int serviceId) {
    return GroupFactory::serviceIdToString(serviceId);
}

} // namespace services
} // namespace iuim