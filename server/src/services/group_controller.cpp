#include "../../include/services/group_controller.h"
#include "../../include/services/group_repository.h"
#include "../../include/services/group_service.h"
#include "../../include/utils/logger.h"

namespace iuim {
namespace services {

GroupController::GroupController(std::shared_ptr<IGroupService> groupService) 
    : groupService_(groupService) {
    if (!groupService_) {
        throw std::invalid_argument("GroupService cannot be null");
    }
    iuim::utils::Logger::getInstance().logInfo("GroupController initialized");
}

void GroupController::handleCreateGroup(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json requestJson;
    if (!parseRequestJson(req.body, requestJson, res)) {
        return;
    }
    
    // 验证必要字段
    std::vector<std::string> requiredFields = {"creator_id", "group_name", "service_id"};
    if (!validateRequiredFields(requestJson, requiredFields, res)) {
        return;
    }
    
    try {
        int creatorId = requestJson["creator_id"];
        std::string groupName = requestJson["group_name"];
        int serviceId = requestJson["service_id"];
        std::string description = requestJson.value("description", "");
        
        logRequest("CreateGroup", requestJson);
        
        nlohmann::json response = groupService_->createGroup(creatorId, groupName, serviceId, description);
        
        logResponse("CreateGroup", response);
        sendJsonResponse(res, response);
        
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in handleCreateGroup: " + std::string(e.what()));
        sendErrorResponse(res, 500, "Internal server error");
    }
}

void GroupController::handleJoinGroup(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json requestJson;
    if (!parseRequestJson(req.body, requestJson, res)) {
        return;
    }
    
    // 验证必要字段
    std::vector<std::string> requiredFields = {"user_id", "group_id", "service_id", "join_type"};
    if (!validateRequiredFields(requestJson, requiredFields, res)) {
        return;
    }
    
    try {
        int userId = requestJson["user_id"];
        int groupId = requestJson["group_id"];
        int serviceId = requestJson["service_id"];
        int joinType = requestJson["join_type"];
        
        logRequest("JoinGroup", requestJson);
        
        nlohmann::json response = groupService_->joinGroup(userId, groupId, serviceId, joinType);
        
        logResponse("JoinGroup", response);
        sendJsonResponse(res, response);
        
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in handleJoinGroup: " + std::string(e.what()));
        sendErrorResponse(res, 500, "Internal server error");
    }
}

void GroupController::handleQuitGroup(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json requestJson;
    if (!parseRequestJson(req.body, requestJson, res)) {
        return;
    }
    
    // 验证必要字段
    std::vector<std::string> requiredFields = {"user_id", "group_id", "service_id"};
    if (!validateRequiredFields(requestJson, requiredFields, res)) {
        return;
    }
    
    try {
        int userId = requestJson["user_id"];
        int groupId = requestJson["group_id"];
        int serviceId = requestJson["service_id"];
        
        logRequest("QuitGroup", requestJson);
        
        nlohmann::json response = groupService_->quitGroup(userId, groupId, serviceId);
        
        logResponse("QuitGroup", response);
        sendJsonResponse(res, response);
        
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in handleQuitGroup: " + std::string(e.what()));
        sendErrorResponse(res, 500, "Internal server error");
    }
}

void GroupController::handleGetGroupMembers(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json requestJson;
    if (!parseRequestJson(req.body, requestJson, res)) {
        return;
    }
    
    // 验证必要字段
    std::vector<std::string> requiredFields = {"group_id", "service_id"};
    if (!validateRequiredFields(requestJson, requiredFields, res)) {
        return;
    }
    
    try {
        int groupId = requestJson["group_id"];
        int serviceId = requestJson["service_id"];
        
        logRequest("GetGroupMembers", requestJson);
        
        nlohmann::json response = groupService_->getGroupMembers(groupId, serviceId);
        
        logResponse("GetGroupMembers", response);
        sendJsonResponse(res, response);
        
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in handleGetGroupMembers: " + std::string(e.what()));
        sendErrorResponse(res, 500, "Internal server error");
    }
}

void GroupController::handleGetUserGroups(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json requestJson;
    if (!parseRequestJson(req.body, requestJson, res)) {
        return;
    }
    
    // 验证必要字段
    std::vector<std::string> requiredFields = {"user_id", "service_id"};
    if (!validateRequiredFields(requestJson, requiredFields, res)) {
        return;
    }
    
    try {
        int userId = requestJson["user_id"];
        int serviceId = requestJson["service_id"];
        
        logRequest("GetUserGroups", requestJson);
        
        nlohmann::json response = groupService_->getUserGroups(userId, serviceId);
        
        logResponse("GetUserGroups", response);
        sendJsonResponse(res, response);
        
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Exception in handleGetUserGroups: " + std::string(e.what()));
        sendErrorResponse(res, 500, "Internal server error");
    }
}

// 静态方法实现 - 用于与现有handler兼容
GroupController& GroupController::getInstance() {
    static std::shared_ptr<IGroupRepository> repository = std::make_shared<GroupRepository>();
    static std::shared_ptr<IGroupService> service = std::make_shared<GroupService>(repository);
    static GroupController instance(service);
    return instance;
}

void GroupController::handleGroupCreate(const httplib::Request& req, httplib::Response& res) {
    getInstance().handleCreateGroup(req, res);
}

void GroupController::handleGroupJoin(const httplib::Request& req, httplib::Response& res) {
    getInstance().handleJoinGroup(req, res);
}

void GroupController::handleGroupQuit(const httplib::Request& req, httplib::Response& res) {
    getInstance().handleQuitGroup(req, res);
}

void GroupController::handleGroupMembers(const httplib::Request& req, httplib::Response& res) {
    getInstance().handleGetGroupMembers(req, res);
}

void GroupController::handleGroupList(const httplib::Request& req, httplib::Response& res) {
    getInstance().handleGetUserGroups(req, res);
}

// 辅助方法实现
bool GroupController::parseRequestJson(const std::string& body, nlohmann::json& json, httplib::Response& res) {
    try {
        json = nlohmann::json::parse(body);
        return true;
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Failed to parse JSON: " + std::string(e.what()));
        sendErrorResponse(res, 400, "Invalid JSON format");
        return false;
    }
}

bool GroupController::validateRequiredFields(const nlohmann::json& json, const std::vector<std::string>& fields, httplib::Response& res) {
    for (const auto& field : fields) {
        if (!json.contains(field)) {
            sendErrorResponse(res, 400, "Missing required field: " + field);
            return false;
        }
    }
    return true;
}

void GroupController::sendJsonResponse(httplib::Response& res, const nlohmann::json& json) {
    res.set_content(json.dump(), "application/json");
}

void GroupController::sendErrorResponse(httplib::Response& res, int code, const std::string& message) {
    nlohmann::json errorJson;
    errorJson["code"] = code;
    errorJson["message"] = message;
    res.set_content(errorJson.dump(), "application/json");
}

void GroupController::logRequest(const std::string& operation, const nlohmann::json& requestData) {
    iuim::utils::Logger::getInstance().logInfo("GroupController::" + operation + " - Request: " + requestData.dump());
}

void GroupController::logResponse(const std::string& operation, const nlohmann::json& responseData) {
    iuim::utils::Logger::getInstance().logInfo("GroupController::" + operation + " - Response: " + responseData.dump());
}

} // namespace services
} // namespace iuim