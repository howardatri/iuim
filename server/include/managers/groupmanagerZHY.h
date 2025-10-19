#pragma once
#include "../utils/database_manager.h"
#include <string>

namespace iuim {
namespace managers {

class GroupManagerZHY {
public:
    static bool createGroup(int creatorId, const std::string& groupName, int serviceId, const std::string& description);
    static bool joinGroup(int userId, int groupId, int serviceId, int role);
    static bool quitGroup(int userId, int groupId, int serviceId);
    static bool queryGroupMembers(int groupId, int serviceId, std::string& jsonResult);
    static bool getUserGroups(int userId, int serviceId, std::string& jsonResult);
};

} // namespace managers
} // namespace iuim