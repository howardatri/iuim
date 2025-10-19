#pragma once
#include "../utils/database_manager.h"
#include <string>

namespace iuim {
namespace managers {

class ServiceManagerZHY {
public:
    static bool addUserServic(int userId, int serviceId);
    static bool removeUserService(int userId, int serviceId);
    static bool queryUserServices(int userId, std::string& jsonResult);
};

} // namespace managers
} // namespace iuim