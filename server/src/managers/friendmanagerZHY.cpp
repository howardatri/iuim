#include "../../include/managers/friendmanagerZHY.h"

namespace iuim {
namespace managers {

bool FriendManagerZHY::addFriend(int userId, int friendId, int serviceId, const std::string& remark) {
    return utils::DatabaseManager::getInstance().addFriend(userId, friendId, serviceId, remark);
}

bool FriendManagerZHY::deleteFriend(int userId, int friendId, int serviceId) {
    return utils::DatabaseManager::getInstance().deleteFriend(userId, friendId, serviceId);
}

bool FriendManagerZHY::queryFriends(int userId, int serviceId, std::string& jsonResult) {
    return utils::DatabaseManager::getInstance().queryFriends(userId, serviceId, jsonResult);
}

bool FriendManagerZHY::searchUsers(const std::string& keyword, int serviceId, std::string& jsonResult) {
    return utils::DatabaseManager::getInstance().searchUsers(keyword,jsonResult);
}

} // namespace managers
} // namespace iuim