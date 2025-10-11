#include "../../include/services/friend_handler.h"
#include "../../include/utils/database_manager.h"
#include "../../include/utils/logger.h"
#include "../../../third_party/nlohmann/json.hpp"

namespace iuim {
namespace services {

using json = nlohmann::json;
using namespace iuim::utils;

void handleFriendAdd(const httplib::Request& req, httplib::Response& res) {
    Logger::getInstance().logInfo("Received friend add request");
    
    json responseJson;
    
    try {
        // 解析请求体
        json requestJson = json::parse(req.body);
        
        // 验证请求参数
        if (!requestJson.contains("user_id") || !requestJson.contains("friend_id") || 
            !requestJson.contains("service_id")) {
            responseJson["code"] = 400;
            responseJson["message"] = "Missing required parameters";
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        int userId = requestJson["user_id"].get<int>();
        int friendId = requestJson["friend_id"].get<int>();
        int serviceId = requestJson["service_id"].get<int>();
        std::string remark = requestJson.value("remark", "");
        
        // 调用数据库管理器添加好友
        bool success = DatabaseManager::getInstance().addFriend(userId, friendId, serviceId, remark);
        
        if (success) {
            responseJson["code"] = 200;
            responseJson["message"] = "Friend added successfully";
        } else {
            responseJson["code"] = 500;
            responseJson["message"] = "Failed to add friend";
        }
    } catch (const std::exception& e) {
        Logger::getInstance().logError("Error in handleFriendAdd: " + std::string(e.what()));
        responseJson["code"] = 500;
        responseJson["message"] = "Internal server error: " + std::string(e.what());
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

void handleFriendDelete(const httplib::Request& req, httplib::Response& res) {
    Logger::getInstance().logInfo("Received friend delete request");
    
    json responseJson;
    
    try {
        // 解析请求体
        json requestJson = json::parse(req.body);
        
        // 验证请求参数
        if (!requestJson.contains("user_id") || !requestJson.contains("friend_id") || 
            !requestJson.contains("service_id")) {
            responseJson["code"] = 400;
            responseJson["message"] = "Missing required parameters";
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        int userId = requestJson["user_id"].get<int>();
        int friendId = requestJson["friend_id"].get<int>();
        int serviceId = requestJson["service_id"].get<int>();
        
        // 调用数据库管理器删除好友
        bool success = DatabaseManager::getInstance().deleteFriend(userId, friendId, serviceId);
        
        if (success) {
            responseJson["code"] = 200;
            responseJson["message"] = "Friend deleted successfully";
        } else {
            responseJson["code"] = 500;
            responseJson["message"] = "Failed to delete friend";
        }
    } catch (const std::exception& e) {
        Logger::getInstance().logError("Error in handleFriendDelete: " + std::string(e.what()));
        responseJson["code"] = 500;
        responseJson["message"] = "Internal server error: " + std::string(e.what());
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

void handleFriendQuery(const httplib::Request& req, httplib::Response& res) {
    Logger::getInstance().logInfo("Received friend query request");
    
    json responseJson;
    
    try {
        // 解析请求体
        json requestJson = json::parse(req.body);
        
        // 验证请求参数
        if (!requestJson.contains("user_id") || !requestJson.contains("service_id")) {
            responseJson["code"] = 400;
            responseJson["message"] = "Missing required parameters";
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        int userId = requestJson["user_id"].get<int>();
        int serviceId = requestJson["service_id"].get<int>();
        
        // 查询好友列表
        std::string jsonResult;
        bool success = DatabaseManager::getInstance().queryFriends(userId, serviceId, jsonResult);
        
        if (success) {
            // 解析查询结果
            json friendsData = json::parse(jsonResult);
            
            // 构建响应
            responseJson["code"] = 0;
            responseJson["message"] = "Friends queried successfully";
            responseJson["data"] = friendsData;
        } else {
            responseJson["code"] = 500;
            responseJson["message"] = "Failed to query friends";
        }
    } catch (const std::exception& e) {
        Logger::getInstance().logError("Error in handleFriendQuery: " + std::string(e.what()));
        responseJson["code"] = 500;
        responseJson["message"] = "Internal server error: " + std::string(e.what());
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

// 处理用户搜索请求
void handleUserSearch(const httplib::Request& req, httplib::Response& res) {
    try {
        auto json_body = nlohmann::json::parse(req.body);
        std::string keyword = json_body.value("keyword", "");
        
        if (keyword.empty()) {
            nlohmann::json response = {
                {"code", 400},
                {"message", "Keyword is required"}
            };
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        // 调用数据库搜索用户
        std::string searchResult;
        if (iuim::utils::DatabaseManager::getInstance().searchUsers(keyword, searchResult)) {
            nlohmann::json response = {
                {"code", 0},
                {"message", "success"},
                {"data", {
                    {"users", nlohmann::json::parse(searchResult)}
                }}
            };
            res.set_content(response.dump(), "application/json");
        } else {
            nlohmann::json response = {
                {"code", 500},
                {"message", "Search failed"}
            };
            res.set_content(response.dump(), "application/json");
        }
        
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("User search error: " + std::string(e.what()));
        
        nlohmann::json response = {
            {"code", 400},
            {"message", "Invalid request format"},
            {"error", e.what()}
        };
        res.set_content(response.dump(), "application/json");
    }
}

} // namespace services
} // namespace iuim