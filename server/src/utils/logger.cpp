#include "../../include/utils/logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace iuim {
namespace utils {

// 初始化静态互斥锁
std::mutex Logger::logMutex;

// 记录INFO级别日志
void Logger::logInfo(const std::string& message) {
    log(LogLevel::INFO, message);
}

// 记录ERROR级别日志
void Logger::logError(const std::string& message) {
    log(LogLevel::ERR, message);
}

// 内部日志记录方法
void Logger::log(LogLevel level, const std::string& message) {
    // 锁定互斥锁，确保线程安全
    std::lock_guard<std::mutex> lock(logMutex);
    
    // 获取当前时间戳
    std::string timestamp = getCurrentTimestamp();
    
    // 根据日志级别确定标签
    std::string levelTag;
    switch (level) {
        case LogLevel::INFO:
            levelTag = "INFO";
            break;
        case LogLevel::ERR:
            levelTag = "ERROR";
            break;
    }
    
    // 输出格式化的日志消息
    std::cout << "[" << timestamp << "] [" << levelTag << "] " << message << std::endl;
}

// 获取当前时间戳字符串
std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    // 获取毫秒部分
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

} // namespace utils
} // namespace iuim