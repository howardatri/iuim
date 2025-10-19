#pragma once
#include "../base/basegroupZHY.h"

namespace iuim {
namespace models {

// QQ群
class QQGroup : public BaseGroup {
public:
    QQGroup(int id, const std::string& name, const std::string& description)
        : BaseGroup(id, name, description) {}
    
    // 未来可以添加QQ群特有的方法, 例如获取群等级
};

} // namespace models
} // namespace iuim
