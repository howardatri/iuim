#include "../../include/services/user_service.h"
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
    
    std::cout << "UserService starting on port 50051..." << std::endl;
    server_->listen("0.0.0.0", 50051);
}

} // namespace services
} // namespace iuim