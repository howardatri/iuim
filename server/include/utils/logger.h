#ifndef IUIM_UTILS_LOGGER_H
#define IUIM_UTILS_LOGGER_H

#include <string>
#include <mutex>
#include <chrono>

namespace iuim {
namespace utils {

// 日志级别枚举
enum class LogLevel {
    INFO,
    ERR
};

// 线程安全的日志工具类
class Logger {
public:
    // 获取单例实例
    static Logger& getInstance();
    
    // 禁用拷贝构造和赋值操作
    Logger(const Logger&) = delete;
    void operator=(const Logger&) = delete;
    
    // 记录INFO级别日志
    void logInfo(const std::string& message);
    
    // 记录ERROR级别日志
    void logError(const std::string& message);

private:
    // 私有构造函数
    Logger() = default;
    
    // 内部日志记录方法
    void log(LogLevel level, const std::string& message);
    
    // 获取当前时间戳字符串
    static std::string getCurrentTimestamp();
    
    // 互斥锁，确保线程安全
    static std::mutex logMutex;
};

} // namespace utils
} // namespace iuim

#endif // IUIM_UTILS_LOGGER_H