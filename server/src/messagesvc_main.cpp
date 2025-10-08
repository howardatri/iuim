#include <iostream>
#include "../../third_party/httplib/httplib.h"
#include "../include/utils/database_manager.h"
#include "../include/utils/logger.h"
#include "../include/services/message_handler.h"

int main() {
    try {
        // 初始化日志系统
        iuim::utils::Logger::getInstance().logInfo("Starting MsgSVC microservice...");
        
        // 初始化数据库连接
        iuim::utils::DatabaseManager& dbManager = iuim::utils::DatabaseManager::getInstance();
        iuim::utils::Logger::getInstance().logInfo("Database initialized successfully");
        
        // 创建HTTP服务器
        httplib::Server server;
        
        // 设置CORS头
        server.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            return httplib::Server::HandlerResponse::Unhandled;
        });
        
        // 处理OPTIONS请求
        server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
        });
        
        // 注册消息发送路由 - POST /send
        server.Post("/send", [](const httplib::Request& req, httplib::Response& res) {
            iuim::services::handleMessageSend(req, res);
        });
        
        // 注册消息历史查询路由 - POST /history
        server.Post("/history", [](const httplib::Request& req, httplib::Response& res) {
            iuim::services::handleMessageHistory(req, res);
        });
        
        // 健康检查接口 - GET /health
        server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
            res.set_header("Content-Type", "application/json");
            res.body = R"({"code": 200, "message": "MsgSVC is running", "data": {"service": "MsgSVC", "status": "healthy"}})";
            res.status = 200;
        });
        
        // 设置服务器错误处理
        server.set_error_handler([](const httplib::Request&, httplib::Response& res) {
            res.set_header("Content-Type", "application/json");
            res.body = R"({"code": 500, "message": "Internal server error", "data": null})";
        });
        
        // 设置异常处理
        server.set_exception_handler([](const httplib::Request&, httplib::Response& res, std::exception_ptr ep) {
            res.set_header("Content-Type", "application/json");
            res.body = R"({"code": 500, "message": "Server exception occurred", "data": null})";
            res.status = 500;
            
            try {
                std::rethrow_exception(ep);
            } catch (const std::exception& e) {
                iuim::utils::Logger::getInstance().logError("Server exception: " + std::string(e.what()));
            }
        });
        
        // 启动服务器监听50052端口
        const int port = 50052;
        iuim::utils::Logger::getInstance().logInfo("MsgSVC server starting on port " + std::to_string(port));
        
        std::cout << "MsgSVC microservice is starting..." << std::endl;
        std::cout << "Server will listen on port " << port << std::endl;
        std::cout << "Available endpoints:" << std::endl;
        std::cout << "  POST /send - Send message" << std::endl;
        std::cout << "  POST /history - Get message history" << std::endl;
        std::cout << "  GET /health - Health check" << std::endl;
        std::cout << "Press Ctrl+C to stop the server." << std::endl;
        
        if (!server.listen("0.0.0.0", port)) {
            iuim::utils::Logger::getInstance().logError("Failed to start server on port " + std::to_string(port));
            std::cerr << "Failed to start server on port " << port << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        iuim::utils::Logger::getInstance().logError("Fatal error in MsgSVC main: " + std::string(e.what()));
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}