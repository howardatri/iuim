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
            responseJson["code"] = 200;
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

} // namespace services
} // namespace iuim