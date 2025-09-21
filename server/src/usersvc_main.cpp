#include "../include/services/user_service.h"
#include "../include/utils/logger.h"
#include <iostream>

int main() {
    iuim::utils::Logger::logInfo("Starting IUIM UserSVC...");
    
    iuim::services::UserService userService;
    iuim::utils::Logger::logInfo("Initializing UserService");
    
    userService.start();
    
    return 0;
}