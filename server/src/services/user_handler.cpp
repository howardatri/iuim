#include "../../include/services/user_handler.h"
#include "../../include/utils/database_manager.h"
#include "../../include/utils/logger.h"
#include "../../../third_party/nlohmann/json.hpp"
#include <iostream>
#include <memory>

using json = nlohmann::json;
using namespace iuim::utils;

namespace iuim {
namespace services {

void handleRegister(const httplib::Request& req, httplib::Response& res) {
    Logger::getInstance().logInfo("Handling register request");
    
    json responseJson;

    // 解析请求体JSON
    try {
        json requestJson = json::parse(req.body);
        
        // 验证必要字段
        if (!requestJson.contains("username") || !requestJson.contains("password")) {
            responseJson = {
                {"code", 400},
                {"message", "缺少必要字段"}
            };
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        std::string username = requestJson["username"];
        std::string password = requestJson["password"];
        std::string nickname = requestJson.value("nickname", username);
        std::string email = requestJson.value("email", "");
        
        // 注册用户
        bool success = DatabaseManager::getInstance().registerUser(username, password, nickname, email);
        
        if (success) {
            Logger::getInstance().logInfo("User registered successfully: " + username);
            responseJson = {
                {"code", 0},
                {"message", "注册成功"},
                {"data", {
                    {"username", username},
                    {"nickname", nickname}
                }}
            };
        } else {
            Logger::getInstance().logError("Failed to register user: " + username);
            responseJson = {
                {"code", 400},
                {"message", "用户名已存在"}
            };
        }
    } catch (const std::exception& e) {
        Logger::getInstance().logError(std::string("Register error: ") + e.what());
        responseJson = {
            {"code", 500},
            {"message", std::string("服务器错误: ") + e.what()}
        };
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

void handleLogin(const httplib::Request& req, httplib::Response& res) {
    Logger::getInstance().logInfo("Handling login request");

    // 解析请求体JSON
    json responseJson;
    try {
        json requestJson = json::parse(req.body);
        
        // 验证必要字段
        if (!requestJson.contains("username") || !requestJson.contains("password")) {
            responseJson = {
                {"code", 400},
                {"message", "缺少必要字段"}
            };
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        std::string username = requestJson["username"];
        std::string password = requestJson["password"];
        
        // 验证登录
        std::string nickname, email;
        bool success = DatabaseManager::getInstance().verifyLogin(username, password, nickname, email);
        
        if (success) {
            int userId = DatabaseManager::getInstance().getUserIdByUsername(username); // 确保这一步已实现

            if (userId != -1) { // 假设找不到用户时返回-1
                Logger::getInstance().logInfo("User login successful: " + username);
                responseJson = {
                    {"code", 0},
                    {"message", "登录成功"},
                    {"data", {
                        {"user_info", {
                            {"user_id", userId},
                            {"username", username},
                            {"nickname", nickname},
                            {"email", email}
                        }}
                    }}
                };
            } else {
                Logger::getInstance().logError("Could not retrieve user_id for: " + username);
                responseJson = {
                    {"code", 500},
                    {"message", "服务器内部错误：无法获取用户信息"}
                };
            }
        } else { // ...原有的登录失败逻辑...
            Logger::getInstance().logError("User login failed: " + username);
            responseJson = {
                {"code", 401},
                {"message", "用户名或密码错误"}
            };
        }
    } catch (const std::exception& e) {
        Logger::getInstance().logError(std::string("Login error: ") + e.what());
        responseJson = {
            {"code", 500},
            {"message", std::string("服务器错误: ") + e.what()}
        };
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

} // namespace services
} // namespace iuim