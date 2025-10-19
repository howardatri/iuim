#pragma once
#include "../base/basegroupZHY.h"

namespace iuim {
namespace models {

// 微博群
class WeiboGroup : public BaseGroup {
public:
    WeiboGroup(int id, const std::string& name, const std::string& description)
        : BaseGroup(id, name, description) {}
};

} // namespace models
} // namespace iuim
