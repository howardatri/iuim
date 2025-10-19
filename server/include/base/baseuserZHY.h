#pragma once
#include <string>
#include <vector>

namespace iuim {
namespace models {

// 用户基类
class BaseUser {
public:
    BaseUser(int id, const std::string& username, const std::string& nickname)
        : id_(id), username_(username), nickname_(nickname) {}
    virtual ~BaseUser() = default;

    int getId() const { return id_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getNickname() const { return nickname_; }

protected:
    int id_;
    std::string username_;
    std::string nickname_;
};

} // namespace models
} // namespace iuim