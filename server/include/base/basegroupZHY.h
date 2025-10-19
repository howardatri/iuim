#pragma once
#include <string>
#include <vector>

namespace iuim {
namespace models {
// 群组基类
class BaseGroup {
public:
    BaseGroup(int id, const std::string& name, const std::string& description)
        : id_(id), name_(name), description_(description) {}
    virtual ~BaseGroup() = default;

    int getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }

protected:
    int id_;
    std::string name_;
    std::string description_;
    // 可以添加成员列表等
};

} // namespace models
} // namespace iuim