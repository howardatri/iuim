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

// 全局数据库管理器实例
static std::unique_ptr<DatabaseManager> g_dbManager = nullptr;

// 初始化数据库管理器
bool initDatabaseManager() {
    if (!g_dbManager) {
        iuim::utils::Logger::logInfo("Initializing database manager");
        g_dbManager = std::make_unique<DatabaseManager>();
        bool result = g_dbManager->initialize();
        if (!result) {
            iuim::utils::Logger::logError("Failed to initialize database manager");
        }
        return result;
    }
    return true;
}

void handleRegister(const httplib::Request& req, httplib::Response& res) {
    iuim::utils::Logger::logInfo("Handling register request");
    
    // 确保数据库已初始化
    if (!g_dbManager && !initDatabaseManager()) {
        iuim::utils::Logger::logError("Database initialization failed during register request");
        json errorResponse = {
            {"code", 500},
            {"message", "数据库初始化失败"}
        };
        res.set_content(errorResponse.dump(), "application/json");
        return;
    }

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
        std::string nickname = requestJson.value("nickname", username);
        std::string email = requestJson.value("email", "");
        
        // 注册用户
        bool success = g_dbManager->registerUser(username, password, nickname, email);
        
        if (success) {
            responseJson = {
                {"code", 0},
                {"message", "注册成功"},
                {"data", {
                    {"username", username},
                    {"nickname", nickname}
                }}
            };
        } else {
            responseJson = {
                {"code", 400},
                {"message", "用户名已存在"}
            };
        }
    } catch (const std::exception& e) {
        responseJson = {
            {"code", 500},
            {"message", std::string("服务器错误: ") + e.what()}
        };
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

void handleLogin(const httplib::Request& req, httplib::Response& res) {
    // 确保数据库已初始化
    if (!g_dbManager && !initDatabaseManager()) {
        json errorResponse = {
            {"code", 500},
            {"message", "数据库初始化失败"}
        };
        res.set_content(errorResponse.dump(), "application/json");
        return;
    }

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
        bool success = g_dbManager->verifyLogin(username, password, nickname, email);
        
        if (success) {
            responseJson = {
                {"code", 0},
                {"message", "登录成功"},
                {"data", {
                    {"username", username},
                    {"nickname", nickname},
                    {"email", email}
                }}
            };
        } else {
            responseJson = {
                {"code", 401},
                {"message", "用户名或密码错误"}
            };
        }
    } catch (const std::exception& e) {
        responseJson = {
            {"code", 500},
            {"message", std::string("服务器错误: ") + e.what()}
        };
    }
    
    res.set_content(responseJson.dump(), "application/json");
}

} // namespace services
} // namespace iuim