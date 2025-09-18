#ifndef IUIM_SERVICES_USER_SERVICE_H
#define IUIM_SERVICES_USER_SERVICE_H

#include <memory>

namespace httplib {
    class Server;
}

namespace iuim {
namespace services {

class UserService {
public:
    UserService();
    ~UserService();
    
    void start();
    
private:
    std::unique_ptr<httplib::Server> server_;
};

} // namespace services
} // namespace iuim

#endif // IUIM_SERVICES_USER_SERVICE_H