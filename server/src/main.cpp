#include "../include/services/user_service.h"
#include <iostream>

int main() {
    std::cout << "Starting IUIM UserSVC..." << std::endl;
    
    iuim::services::UserService userService;
    userService.start();
    
    return 0;
}