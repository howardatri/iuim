#include "../../include/base/UserBase.h"

UserBase::UserBase() : id(""), nickname(""), password("") {}

UserBase::UserBase(const string& id, const string& nickname, const string& password)
    : id(id), nickname(nickname), password(password) {}

UserBase::~UserBase() {}

string UserBase::GetID() const {
    return id;
}

void UserBase::SetNickname(const string& name) {
    nickname = name;
}

string UserBase::GetNickname() const {
    return nickname;
}

bool UserBase::CheckPassword(const string& pwd) const {
    return password == pwd;
}

bool UserBase::AddFriend(const string& friendID) {
    // 检查是否已经是好友
    for (const auto& id : friendList) {
        if (id == friendID) {
            return false;
        }
    }
    friendList.push_back(friendID);
    return true;
}

bool UserBase::RemoveFriend(const string& friendID) {
    for (auto it = friendList.begin(); it != friendList.end(); ++it) {
        if (*it == friendID) {
            friendList.erase(it);
            return true;
        }
    }
    return false;
}

bool UserBase::ModifyFriend(const string& friendID, const string& newNickname) {
    // 在实际应用中，这里应该修改好友的备注名
    // 简化版本中，我们只检查好友是否存在
    for (const auto& id : friendList) {
        if (id == friendID) {
            return true;
        }
    }
    return false;
}

vector<string> UserBase::QueryFriends() const {
    return friendList;
}

bool UserBase::JoinGroup(const string& groupID) {
    // 检查是否已经在群组中
    for (const auto& id : groupList) {
        if (id == groupID) {
            return false;
        }
    }
    groupList.push_back(groupID);
    return true;
}

bool UserBase::QuitGroup(const string& groupID) {
    for (auto it = groupList.begin(); it != groupList.end(); ++it) {
        if (*it == groupID) {
            groupList.erase(it);
            return true;
        }
    }
    return false;
}

vector<string> UserBase::QueryGroups() const {
    return groupList;
}