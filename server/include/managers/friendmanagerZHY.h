#pragma once
#include "../utils/database_manager.h"
#include <string>

namespace iuim {
namespace managers {

class FriendManagerZHY {
public:
    static bool addFriend(int userId, int friendId, int serviceId, const std::string& remark);
    static bool deleteFriend(int userId, int friendId, int serviceId);
    static bool queryFriends(int userId, int serviceId, std::string& jsonResult);
    static bool searchUsers(const std::string& keyword, int serviceId, std::string& jsonResult);
};

} // namespace managers
} // namespace iuim