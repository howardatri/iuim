#pragma once
#include "../base/baseuserZHY.h"

namespace iuim {
namespace models {

// 微信用户
class WeChatUser : public BaseUser {
public:
    WeChatUser(int id, const std::string& username, const std::string& nickname)
        : BaseUser(id, username, nickname) {}
};

} // namespace models
} // namespace iuim
