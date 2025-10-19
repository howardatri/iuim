#pragma once
#include "../base/baseuserZHY.h"

namespace iuim {
namespace models {

// 微博用户
class WeiboUser : public BaseUser {
public:
    WeiboUser(int id, const std::string& username, const std::string& nickname)
        : BaseUser(id, username, nickname) {}
};

} // namespace models
} // namespace iuim
