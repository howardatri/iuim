#ifndef IUIM_GROUP_FACTORY_H
#define IUIM_GROUP_FACTORY_H

#include <memory>
#include <string>
#include "../models/qqgroupZHY.h"
#include "../models/wechatgroupZHY.h"
#include "../models/weibogroupZHY.h"

namespace iuim {
namespace services {

// 群组工厂类 - 负责创建不同类型的群组对象
class GroupFactory {
public:
    // 服务类型枚举
    enum class ServiceType {
        QQ = 1,
        WECHAT = 2,
        WEIBO = 3
    };
    
    // 创建群组对象
    static std::shared_ptr<iuim::models::BaseGroup> createGroup(int id, const std::string& name, 
                                                                const std::string& description, 
                                                                ServiceType serviceType);
    
    // 根据服务ID创建群组对象
    static std::shared_ptr<iuim::models::BaseGroup> createGroup(int id, const std::string& name, 
                                                                const std::string& description, 
                                                                int serviceId);
    
    // 服务类型转换
    static ServiceType intToServiceType(int serviceId);
    static int serviceTypeToInt(ServiceType serviceType);
    static std::string serviceTypeToString(ServiceType serviceType);
    static std::string serviceIdToString(int serviceId);
    
    // 验证服务类型
    static bool isValidServiceId(int serviceId);
    static bool isValidServiceType(ServiceType serviceType);
    
private:
    GroupFactory() = default; // 禁止实例化
};

} // namespace services
} // namespace iuim

#endif // IUIM_GROUP_FACTORY_H