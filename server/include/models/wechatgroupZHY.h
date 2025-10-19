#pragma once
#include "../base/basegroupZHY.h"

namespace iuim {
namespace models {

// 微信群
class WeChatGroup : public BaseGroup {
public:
    WeChatGroup(int id, const std::string& name, const std::string& description)
        : BaseGroup(id, name, description) {}
};

} // namespace models
} // namespace iuim
