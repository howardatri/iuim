#include "../../include/services/group_handler.h"
#include "../../include/utils/database_manager.h"
#include "../../include/utils/logger.h"
#include "nlohmann/json.hpp"

namespace iuim {
namespace services {

// 处理加入群组请求
void handleGroupJoin(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json responseJson;
    
    try {
        // 解析请求体
        auto requestJson = nlohmann::json::parse(req.body);
        
        // 验证必要参数
        if (!requestJson.contains("user_id") || !requestJson.contains("group_id") || 
            !requestJson.contains("service_id") || !requestJson.contains("join_type")) {
            responseJson["code"] = 400;
            responseJson["message"] = "Missing required parameters";
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        int userId = requestJson["user_id"];
        int groupId = requestJson["group_id"];
        int serviceId = requestJson["service_id"];
        int joinType = requestJson["join_type"];
        
        // 验证加入类型
        if (joinType != 0 && joinType != 1) {
            responseJson["code"] = 400;
            responseJson["message"] = "Invalid join_type, must be 0 (apply) or 1 (invite)";
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        // 调用数据库管理器执行加入群组操作
        bool success = iuim::utils::DatabaseManager::getInstance().joinGroup(userId, groupId, serviceId, joinType);
        
        if (success) {
            responseJson["code"] = 0;
            responseJson["message"] = "Successfully joined the group";
        } else {
            responseJson["code"] = 500;
            responseJson["message"] = "Failed to join the group";
        }
    } catch (const std::exception& e) {
        responseJson["code"] = 500;
        responseJson["message"] = std::string("Error: ") + e.what();
        iuim::utils::Logger::getInstance().logError("Error in handleGroupJoin: " + std::string(e.what()));
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

// 处理退出群组请求
void handleGroupQuit(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json responseJson;
    
    try {
        // 解析请求体
        auto requestJson = nlohmann::json::parse(req.body);
        
        // 验证必要参数
        if (!requestJson.contains("user_id") || !requestJson.contains("group_id") || 
            !requestJson.contains("service_id")) {
            responseJson["code"] = 400;
            responseJson["message"] = "Missing required parameters";
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        int userId = requestJson["user_id"];
        int groupId = requestJson["group_id"];
        int serviceId = requestJson["service_id"];
        
        // 调用数据库管理器执行退出群组操作
        bool success = iuim::utils::DatabaseManager::getInstance().quitGroup(userId, groupId, serviceId);
        
        if (success) {
            responseJson["code"] = 0;
            responseJson["message"] = "Successfully quit the group";
        } else {
            responseJson["code"] = 500;
            responseJson["message"] = "Failed to quit the group";
        }
    } catch (const std::exception& e) {
        responseJson["code"] = 500;
        responseJson["message"] = std::string("Error: ") + e.what();
        iuim::utils::Logger::getInstance().logError("Error in handleGroupQuit: " + std::string(e.what()));
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

// 处理查询群成员请求 - 修改为与其他handler一致的风格
void handleGroupMembers(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json responseJson;
    
    try {
        // 解析请求体
        auto requestJson = nlohmann::json::parse(req.body);
        
        // 验证必要参数
        if (!requestJson.contains("group_id") || !requestJson.contains("service_id")) {
            responseJson["code"] = 400;
            responseJson["message"] = "Missing required parameters";
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        int groupId = requestJson["group_id"];
        int serviceId = requestJson["service_id"];
        
        // 调用数据库管理器执行查询群成员操作
        std::string jsonResult;
        bool success = iuim::utils::DatabaseManager::getInstance().queryGroupMembers(groupId, serviceId, jsonResult);
        
        if (success) {
            // 解析查询结果并构建标准响应格式
            auto membersData = nlohmann::json::parse(jsonResult);
            
            responseJson["code"] = 0;
            responseJson["message"] = "success";
            responseJson["data"] = {
                {"members", membersData}
            };
        } else {
            responseJson["code"] = 500;
            responseJson["message"] = "Failed to query group members";
        }
    } catch (const std::exception& e) {
        responseJson["code"] = 500;
        responseJson["message"] = std::string("Error: ") + e.what();
        iuim::utils::Logger::getInstance().logError("Error in handleGroupMembers: " + std::string(e.what()));
    }
    
    res.set_content(responseJson.dump(), "application/json");
}
// 处理创建群组请求
void handleGroupCreate(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json responseJson;
    
    try {
        // 解析请求体
        auto requestJson = nlohmann::json::parse(req.body);
        
        // 验证必要参数
        if (!requestJson.contains("creator_id") || !requestJson.contains("group_name") || 
            !requestJson.contains("service_id")) {
            responseJson["code"] = 400;
            responseJson["message"] = "Missing required parameters";
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        int creatorId = requestJson["creator_id"];
        std::string groupName = requestJson["group_name"];
        int serviceId = requestJson["service_id"];
        
        // 获取可选参数
        std::string description = "";
        if (requestJson.contains("description")) {
            description = requestJson["description"];
        }
        
        // 调用数据库管理器执行创建群组操作
        bool success = iuim::utils::DatabaseManager::getInstance().createGroup(creatorId, groupName, serviceId, description);
        
        if (success) {
            responseJson["code"] = 0;
            responseJson["message"] = "Successfully created the group";
        } else {
            responseJson["code"] = 500;
            responseJson["message"] = "Failed to create the group";
        }
    } catch (const std::exception& e) {
        responseJson["code"] = 500;
        responseJson["message"] = std::string("Error: ") + e.what();
        iuim::utils::Logger::getInstance().logError("Error in handleGroupCreate: " + std::string(e.what()));
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

// 处理获取用户群组列表请求
void handleGroupList(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json responseJson;
    
    try {
        // 解析请求体
        auto requestJson = nlohmann::json::parse(req.body);
        
        // 验证必要参数
        if (!requestJson.contains("user_id") || !requestJson.contains("service_id")) {
            responseJson["code"] = 400;
            responseJson["message"] = "Missing required parameters";
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        int userId = requestJson["user_id"];
        int serviceId = requestJson["service_id"];
        
        // 调用数据库管理器执行查询用户群组列表操作
        std::string jsonResult;
        bool success = iuim::utils::DatabaseManager::getInstance().getUserGroups(userId, serviceId, jsonResult);
        
        if (success) {
            // 解析查询结果并构建标准响应格式
            auto groupsData = nlohmann::json::parse(jsonResult);
            
            responseJson["code"] = 0;
            responseJson["message"] = "success";
            responseJson["data"] = {
                {"groups", groupsData}
            };
        } else {
            responseJson["code"] = 500;
            responseJson["message"] = "Failed to query user groups";
        }
    } catch (const std::exception& e) {
        responseJson["code"] = 500;
        responseJson["message"] = std::string("Error: ") + e.what();
        iuim::utils::Logger::getInstance().logError("Error in handleGroupList: " + std::string(e.what()));
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

} // namespace services
} // namespace iuim