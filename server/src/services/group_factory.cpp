#include "../../include/services/group_factory.h"
#include "../../include/utils/logger.h"

namespace iuim {
namespace services {

std::shared_ptr<iuim::models::BaseGroup> GroupFactory::createGroup(int id, const std::string& name, 
                                                                   const std::string& description, 
                                                                   ServiceType serviceType) {
    switch (serviceType) {
        case ServiceType::QQ:
            return std::make_shared<iuim::models::QQGroup>(id, name, description);
        case ServiceType::WECHAT:
            return std::make_shared<iuim::models::WeChatGroup>(id, name, description);
        case ServiceType::WEIBO:
            return std::make_shared<iuim::models::WeiboGroup>(id, name, description);
        default:
            iuim::utils::Logger::getInstance().logError("Invalid service type: " + std::to_string(static_cast<int>(serviceType)));
            return nullptr;
    }
}

std::shared_ptr<iuim::models::BaseGroup> GroupFactory::createGroup(int id, const std::string& name, 
                                                                   const std::string& description, 
                                                                   int serviceId) {
    ServiceType serviceType = intToServiceType(serviceId);
    return createGroup(id, name, description, serviceType);
}

GroupFactory::ServiceType GroupFactory::intToServiceType(int serviceId) {
    switch (serviceId) {
        case 1:
            return ServiceType::QQ;
        case 2:
            return ServiceType::WECHAT;
        case 3:
            return ServiceType::WEIBO;
        default:
            iuim::utils::Logger::getInstance().logError("Unknown service ID: " + std::to_string(serviceId) + ", defaulting to QQ");
            return ServiceType::QQ;
    }
}

int GroupFactory::serviceTypeToInt(ServiceType serviceType) {
    return static_cast<int>(serviceType);
}

std::string GroupFactory::serviceTypeToString(ServiceType serviceType) {
    switch (serviceType) {
        case ServiceType::QQ:
            return "QQ";
        case ServiceType::WECHAT:
            return "WeChat";
        case ServiceType::WEIBO:
            return "Weibo";
        default:
            return "Unknown";
    }
}

std::string GroupFactory::serviceIdToString(int serviceId) {
    ServiceType serviceType = intToServiceType(serviceId);
    return serviceTypeToString(serviceType);
}

bool GroupFactory::isValidServiceId(int serviceId) {
    return serviceId >= 1 && serviceId <= 3;
}

bool GroupFactory::isValidServiceType(ServiceType serviceType) {
    return serviceType == ServiceType::QQ || 
           serviceType == ServiceType::WECHAT || 
           serviceType == ServiceType::WEIBO;
}

} // namespace services
} // namespace iuim