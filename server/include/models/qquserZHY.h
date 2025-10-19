#pragma once
#include "../base/baseuserZHY.h"

namespace iuim {
namespace models {

// QQ用户
class QQUser : public BaseUser {
public:
    QQUser(int id, const std::string& username, const std::string& nickname)
        : BaseUser(id, username, nickname) {}
    
    // 未来可以添加QQ用户特有的方法, 例如获取QQ等级
};

} // namespace models
} // namespace iuim
