#include "../../include/services/message_handler.h"
#include "../../include/utils/database_manager.h"
#include "../../include/utils/logger.h"
#include "../../../third_party/nlohmann/json.hpp"
#include <iostream>

namespace iuim {
namespace services {

void handleMessageSend(const httplib::Request& req, httplib::Response& res) {
    res.set_header("Content-Type", "application/json");
    
    try {
        // 解析请求体
        nlohmann::json requestJson = nlohmann::json::parse(req.body);
        
        // 验证必需参数
        if (!requestJson.contains("sender_id") || !requestJson.contains("receiver_id") || 
            !requestJson.contains("type") || !requestJson.contains("content") || 
            !requestJson.contains("service_id")) {
            nlohmann::json response;
            response["code"] = 400;
            response["message"] = "Missing required parameters";
            response["data"] = nullptr;
            res.status = 400;
            res.body = response.dump();
            return;
        }
        
        // 提取参数
        int senderId = requestJson["sender_id"];
        int receiverId = requestJson["receiver_id"];
        int type = requestJson["type"];
        std::string content = requestJson["content"];
        int serviceId = requestJson["service_id"];
        
        // 验证参数有效性
        if (senderId <= 0 || receiverId <= 0 || serviceId <= 0) {
            nlohmann::json response;
            response["code"] = 400;
            response["message"] = "Invalid parameter values";
            response["data"] = nullptr;
            res.status = 400;
            res.body = response.dump();
            return;
        }
        
        if (type != 0 && type != 1) {
            nlohmann::json response;
            response["code"] = 400;
            response["message"] = "Invalid message type (must be 0 for private or 1 for group)";
            response["data"] = nullptr;
            res.status = 400;
            res.body = response.dump();
            return;
        }
        
        if (content.empty()) {
            nlohmann::json response;
            response["code"] = 400;
            response["message"] = "Message content cannot be empty";
            response["data"] = nullptr;
            res.status = 400;
            res.body = response.dump();
            return;
        }
        
        // 调用数据库管理器发送消息
        utils::DatabaseManager& dbManager = utils::DatabaseManager::getInstance();
        bool success = dbManager.sendMessage(senderId, receiverId, type, content, serviceId);
        
        nlohmann::json response;
        if (success) {
            response["code"] = 0;
            response["message"] = "Message sent successfully";
            response["data"] = {
                {"sender_id", senderId},
                {"receiver_id", receiverId},
                {"type", type},
                {"service_id", serviceId}
            };
            res.status = 200;
        } else {
            response["code"] = 500;
            response["message"] = "Failed to send message";
            response["data"] = nullptr;
            res.status = 500;
        }
        
        res.body = response.dump();
        
    } catch (const nlohmann::json::exception& e) {
        nlohmann::json response;
        response["code"] = 400;
        response["message"] = "Invalid JSON format";
        response["data"] = nullptr;
        res.status = 400;
        res.body = response.dump();
        
        utils::Logger::getInstance().logError("JSON parsing error in handleMessageSend: " + std::string(e.what()));
    } catch (const std::exception& e) {
        nlohmann::json response;
        response["code"] = 500;
        response["message"] = "Internal server error";
        response["data"] = nullptr;
        res.status = 500;
        res.body = response.dump();
        
        utils::Logger::getInstance().logError("Error in handleMessageSend: " + std::string(e.what()));
    }
}

void handleMessageHistory(const httplib::Request& req, httplib::Response& res) {
    res.set_header("Content-Type", "application/json");
    
    try {
        // 解析请求体
        nlohmann::json requestJson = nlohmann::json::parse(req.body);
        
        // 验证必需参数
        if (!requestJson.contains("user_id") || !requestJson.contains("target_id") || 
            !requestJson.contains("type") || !requestJson.contains("service_id")) {
            nlohmann::json response;
            response["code"] = 400;
            response["message"] = "Missing required parameters";
            response["data"] = nullptr;
            res.status = 400;
            res.body = response.dump();
            return;
        }
        
        // 提取参数
        int userId = requestJson["user_id"];
        int targetId = requestJson["target_id"];
        int type = requestJson["type"];
        int serviceId = requestJson["service_id"];
        
        // 可选参数，设置默认值
        int page = requestJson.contains("page") ? requestJson["page"].get<int>() : 1;
        int pageSize = requestJson.contains("page_size") ? requestJson["page_size"].get<int>() : 20;
        
        // 验证参数有效性
        if (userId <= 0 || targetId <= 0 || serviceId <= 0) {
            nlohmann::json response;
            response["code"] = 400;
            response["message"] = "Invalid parameter values";
            response["data"] = nullptr;
            res.status = 400;
            res.body = response.dump();
            return;
        }
        
        if (type != 0 && type != 1) {
            nlohmann::json response;
            response["code"] = 400;
            response["message"] = "Invalid message type (must be 0 for private or 1 for group)";
            response["data"] = nullptr;
            res.status = 400;
            res.body = response.dump();
            return;
        }
        
        if (page <= 0 || pageSize <= 0 || pageSize > 100) {
            nlohmann::json response;
            response["code"] = 400;
            response["message"] = "Invalid page parameters (page > 0, 0 < page_size <= 100)";
            response["data"] = nullptr;
            res.status = 400;
            res.body = response.dump();
            return;
        }
        
        // 调用数据库管理器查询消息历史
        utils::DatabaseManager& dbManager = utils::DatabaseManager::getInstance();
        std::string jsonResult;
        bool success = dbManager.getMessageHistory(userId, targetId, type, serviceId, page, pageSize, jsonResult);
        
        nlohmann::json response;
        if (success) {
            response["code"] = 0;
            response["message"] = "Message history retrieved successfully";
            
            // 解析数据库返回的JSON结果
            try {
                nlohmann::json historyData = nlohmann::json::parse(jsonResult);
                response["data"] = historyData;
            } catch (const nlohmann::json::exception& e) {
                response["data"] = nlohmann::json::parse(jsonResult);
            }
            
            res.status = 200;
        } else {
            response["code"] = 500;
            response["message"] = "Failed to retrieve message history";
            response["data"] = nullptr;
            res.status = 500;
        }
        
        res.body = response.dump();
        
    } catch (const nlohmann::json::exception& e) {
        nlohmann::json response;
        response["code"] = 400;
        response["message"] = "Invalid JSON format";
        response["data"] = nullptr;
        res.status = 400;
        res.body = response.dump();
        
        utils::Logger::getInstance().logError("JSON parsing error in handleMessageHistory: " + std::string(e.what()));
    } catch (const std::exception& e) {
        nlohmann::json response;
        response["code"] = 500;
        response["message"] = "Internal server error";
        response["data"] = nullptr;
        res.status = 500;
        res.body = response.dump();
        
        utils::Logger::getInstance().logError("Error in handleMessageHistory: " + std::string(e.what()));
    }
}

} // namespace services
} // namespace iuim