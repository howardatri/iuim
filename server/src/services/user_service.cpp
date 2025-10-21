#include "../../include/services/user_service.h"
#include "../../include/services/user_handler.h"
#include "../../include/services/user_controller.h"
#include "../../../third_party/httplib/httplib.h"
#include "../../../third_party/nlohmann/json.hpp"
#include <iostream>
#include <string>

using json = nlohmann::json;

namespace iuim {
namespace services {

UserService::UserService() {
    std::cout << "UserService initialized" << std::endl;
}

UserService::~UserService() {
    if (server_) {
        server_->stop();
    }
}

void UserService::start() {
    server_ = std::make_unique<httplib::Server>();
    
    // 注册健康检查接口
    server_->Post("/health-check", [](const httplib::Request& req, httplib::Response& res) {
        json response = {
            {"code", 0},
            {"message", "UserSVC is healthy"},
            {"service", "UserSVC"}
        };
        
        res.set_content(response.dump(), "application/json");
    });
    
    // 注册用户注册接口 (使用OOP包裹器)
    server_->Post("/register", UserController::handleRegister);
    
    // 注册用户登录接口 (使用OOP包裹器)
    server_->Post("/login", UserController::handleLogin);
    
    // 用户资料更新接口
    server_->Post("/update_profile", [](const httplib::Request& req, httplib::Response& res) {
        iuim::services::handleUpdateProfile(req, res);
    });
    
    // 微信绑定接口
    server_->Post("/bind_wechat", [](const httplib::Request& req, httplib::Response& res) {
        iuim::services::handleBindWechat(req, res);
    });
    
    // 获取用户资料接口
    server_->Post("/get_profile", [](const httplib::Request& req, httplib::Response& res) {
        iuim::services::handleGetProfile(req, res);
    });
    
    std::cout << "UserService starting on port 50051..." << std::endl;
    std::cout << "Registered endpoints: /health-check, /register, /login, /update_profile, /bind_wechat, /get_profile" << std::endl;
    server_->listen("0.0.0.0", 50051);
}

} // namespace services
} // namespace iuim