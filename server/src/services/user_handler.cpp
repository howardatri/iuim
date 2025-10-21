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

void handleUpdateProfile(const httplib::Request& req, httplib::Response& res) {
    Logger::getInstance().logInfo("Handling update profile request");
    
    json responseJson;
    
    try {
        json requestJson = json::parse(req.body);
        
        // 验证必要字段
        if (!requestJson.contains("user_id")) {
            responseJson = {
                {"code", 400},
                {"message", "缺少必要字段: user_id"}
            };
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        int user_id = requestJson["user_id"];
        std::string nickname = requestJson.value("nickname", "");
        std::string birth_date = requestJson.value("birth_date", "");
        std::string location = requestJson.value("location", "");
        
        // 检查是否有字段需要更新
        if (nickname.empty() && birth_date.empty() && location.empty()) {
            responseJson = {
                {"code", 400},
                {"message", "至少需要提供一个要更新的字段"}
            };
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        // 更新用户资料
        bool success = DatabaseManager::getInstance().updateUserProfile(user_id, nickname, birth_date, location);
        
        if (success) {
            Logger::getInstance().logInfo("User profile updated successfully: user_id=" + std::to_string(user_id));
            responseJson = {
                {"code", 0},
                {"message", "用户资料更新成功"},
                {"data", {
                    {"user_id", user_id}
                }}
            };
        } else {
            Logger::getInstance().logError("Failed to update user profile: user_id=" + std::to_string(user_id));
            responseJson = {
                {"code", 500},
                {"message", "更新用户资料失败"}
            };
        }
    } catch (const std::exception& e) {
        Logger::getInstance().logError(std::string("Update profile error: ") + e.what());
        responseJson = {
            {"code", 500},
            {"message", std::string("服务器错误: ") + e.what()}
        };
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

void handleBindWechat(const httplib::Request& req, httplib::Response& res) {
    Logger::getInstance().logInfo("Handling bind wechat request");
    
    json responseJson;
    
    try {
        json requestJson = json::parse(req.body);
        
        // 验证必要字段
        if (!requestJson.contains("user_id") || !requestJson.contains("wechat_id")) {
            responseJson = {
                {"code", 400},
                {"message", "缺少必要字段: user_id, wechat_id"}
            };
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        int user_id = requestJson["user_id"];
        std::string wechat_id = requestJson["wechat_id"];
        
        // 检查微信ID是否可用
        if (!DatabaseManager::getInstance().isWechatIdAvailable(wechat_id)) {
            responseJson = {
                {"code", 409},
                {"message", "微信ID已被其他用户绑定"}
            };
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        // 绑定微信ID
        bool success = DatabaseManager::getInstance().bindWechatId(user_id, wechat_id);
        
        if (success) {
            Logger::getInstance().logInfo("Wechat ID bound successfully: user_id=" + std::to_string(user_id) + ", wechat_id=" + wechat_id);
            responseJson = {
                {"code", 0},
                {"message", "微信绑定成功"},
                {"data", {
                    {"user_id", user_id},
                    {"wechat_id", wechat_id}
                }}
            };
        } else {
            Logger::getInstance().logError("Failed to bind wechat ID: user_id=" + std::to_string(user_id));
            responseJson = {
                {"code", 500},
                {"message", "绑定微信失败"}
            };
        }
    } catch (const std::exception& e) {
        Logger::getInstance().logError(std::string("Bind wechat error: ") + e.what());
        responseJson = {
            {"code", 500},
            {"message", std::string("服务器错误: ") + e.what()}
        };
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

void handleGetProfile(const httplib::Request& req, httplib::Response& res) {
    Logger::getInstance().logInfo("Handling get profile request");
    
    json responseJson;
    
    try {
        json requestJson = json::parse(req.body);
        
        // 验证必要字段
        if (!requestJson.contains("user_id")) {
            responseJson = {
                {"code", 400},
                {"message", "缺少必要字段: user_id"}
            };
            res.set_content(responseJson.dump(), "application/json");
            return;
        }
        
        int user_id = requestJson["user_id"];
        
        // 获取用户完整资料
        std::string profileJson;
        bool success = DatabaseManager::getInstance().getUserFullProfile(user_id, profileJson);
        
        if (success) {
            Logger::getInstance().logInfo("User profile retrieved successfully: user_id=" + std::to_string(user_id));
            
            // 解析数据库返回的JSON并重新包装
            json profileData = json::parse(profileJson);
            responseJson = {
                {"code", 0},
                {"message", "获取用户资料成功"},
                {"data", profileData}
            };
        } else {
            Logger::getInstance().logError("Failed to get user profile: user_id=" + std::to_string(user_id));
            responseJson = {
                {"code", 404},
                {"message", "用户不存在或获取资料失败"}
            };
        }
    } catch (const std::exception& e) {
        Logger::getInstance().logError(std::string("Get profile error: ") + e.what());
        responseJson = {
            {"code", 500},
            {"message", std::string("服务器错误: ") + e.what()}
        };
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

} // namespace services
} // namespace iuim