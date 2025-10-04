#include "../../include/services/service_handler.h"
#include <iostream>

namespace iuim {
namespace services {

using json = nlohmann::json;
using namespace iuim::utils;

// 处理服务激活请求
void handleActivate(const httplib::Request& req, httplib::Response& res) {
    Logger::getInstance().logInfo("Received activate service request");
    
    json responseJson;
    
    try {
        // 解析请求JSON
        json requestJson = json::parse(req.body);
        
        // 验证必要字段
        if (!requestJson.contains("user_id") || !requestJson.contains("service_id")) {
            responseJson = {
                {"code", 400},
                {"message", "Missing required fields: user_id or service_id"}
            };
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        int userId = requestJson["user_id"];
        int serviceId = requestJson["service_id"];
        
        // 激活用户服务
        bool success = DatabaseManager::getInstance().activateUserService(userId, serviceId);
        
        if (success) {
            responseJson = {
                {"code", 0},
                {"message", "success"}
            };
        } else {
            responseJson = {
                {"code", 500},
                {"message", "Failed to activate service"}
            };
        }
    } catch (const std::exception& e) {
        Logger::getInstance().logError("Error in handleActivate: " + std::string(e.what()));
        responseJson = {
            {"code", 500},
            {"message", "Internal server error: " + std::string(e.what())}
        };
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

// 处理服务停用请求
void handleDeactivate(const httplib::Request& req, httplib::Response& res) {
    Logger::getInstance().logInfo("Received deactivate service request");
    
    json responseJson;
    
    try {
        // 解析请求JSON
        json requestJson = json::parse(req.body);
        
        // 验证必要字段
        if (!requestJson.contains("user_id") || !requestJson.contains("service_id")) {
            responseJson = {
                {"code", 400},
                {"message", "Missing required fields: user_id or service_id"}
            };
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        int userId = requestJson["user_id"];
        int serviceId = requestJson["service_id"];
        
        // 停用用户服务
        bool success = DatabaseManager::getInstance().deactivateUserService(userId, serviceId);
        
        if (success) {
            responseJson = {
                {"code", 0},
                {"message", "success"}
            };
        } else {
            responseJson = {
                {"code", 500},
                {"message", "Failed to deactivate service"}
            };
        }
    } catch (const std::exception& e) {
        Logger::getInstance().logError("Error in handleDeactivate: " + std::string(e.what()));
        responseJson = {
            {"code", 500},
            {"message", "Internal server error: " + std::string(e.what())}
        };
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

// 处理查询用户服务请求
void handleQueryUserServices(const httplib::Request& req, httplib::Response& res) {
    Logger::getInstance().logInfo("Received query user services request");
    
    json responseJson;
    
    try {
        // 解析请求JSON
        json requestJson = json::parse(req.body);
        
        // 验证必要字段
        if (!requestJson.contains("user_id")) {
            responseJson = {
                {"code", 400},
                {"message", "Missing required field: user_id"}
            };
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        int userId = requestJson["user_id"];
        
        // 查询用户服务
        std::string servicesJson;
        bool success = DatabaseManager::getInstance().queryUserServices(userId, servicesJson);
        
        if (success) {
            responseJson = {
                {"code", 0},
                {"message", "success"},
                {"data", {
                    {"services", json::parse(servicesJson)}
                }}
            };
        } else {
            responseJson = {
                {"code", 500},
                {"message", "Failed to query user services"}
            };
        }
    } catch (const std::exception& e) {
        Logger::getInstance().logError("Error in handleQueryUserServices: " + std::string(e.what()));
        responseJson = {
            {"code", 500},
            {"message", "Internal server error: " + std::string(e.what())}
        };
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

} // namespace services
} // namespace iuim