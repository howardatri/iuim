#pragma once
#include <string>
#include <vector>

namespace iuim {
namespace models {
// 服务基类 (如QQ, WeChat)
class BaseService {
public:
    BaseService(int id, const std::string& name) : id_(id), name_(name) {}
    virtual ~BaseService() = default;

    int getId() const { return id_; }
    const std::string& getName() const { return name_; }

protected:
    int id_;
    std::string name_;
};

} // namespace models
} // namespace iuim