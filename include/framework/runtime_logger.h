/**
 * @file logger.h
 * @brief 编译时和运行时可控的日志系统
 */

#pragma once

#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <cstdarg>

// === 编译时控制宏 ===
#ifndef LUA_RUNTIME_ENABLE_LOGGING
#define LUA_RUNTIME_ENABLE_LOGGING 1  // 默认启用
#endif

#ifndef LUA_RUNTIME_LOG_LEVEL
#define LUA_RUNTIME_LOG_LEVEL 3       // 0=OFF, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG, 5=TRACE
#endif

namespace lua_runtime {

/**
 * @brief 日志级别枚举
 */
enum class LogLevel {
    OFF = 0,
    ERROR = 1,
    WARN = 2,
    INFO = 3,
    DEBUG = 4,
    TRACE = 5
};

/**
 * @brief 日志系统类
 */
class Logger {
public:
    /**
     * @brief 日志配置结构
     */
    struct Config {
        LogLevel runtime_level = LogLevel::INFO;     // 运行时可调整的级别
        bool enable_file_output = false;            // 文件输出
        bool enable_console_output = true;          // 控制台输出
        std::string log_file_path = "lua_runtime.log";
        bool enable_timestamps = true;              // 时间戳
        bool enable_thread_ids = false;             // 线程ID
        bool enable_color_output = true;            // 彩色输出
        size_t max_log_file_size_mb = 10;           // 最大日志文件大小
        size_t max_log_files = 5;                   // 最大日志文件数量
    };
    
    /**
     * @brief 自定义日志输出器类型
     */
    using LogOutput = std::function<void(LogLevel, const std::string&)>;
    
    /**
     * @brief 配置日志系统
     * @param config 日志配置
     */
    static void configure(const Config& config);
    
    /**
     * @brief 获取当前配置
     * @return 配置引用
     */
    static Config& getConfig();
    
    /**
     * @brief 设置运行时日志级别（受编译时宏限制）
     * @param level 新的日志级别
     */
    static void setRuntimeLevel(LogLevel level);
    
    /**
     * @brief 获取运行时日志级别
     * @return 当前运行时日志级别
     */
    static LogLevel getRuntimeLevel();
    
    /**
     * @brief 添加自定义输出器
     * @param output 输出器函数
     */
    static void addCustomOutput(LogOutput output);
    
    /**
     * @brief 移除所有自定义输出器
     */
    static void removeAllCustomOutputs();
    
    // 日志输出方法（仅在内部使用，外部应使用宏）
    static void error(const char* file, int line, const char* format, ...);
    static void warn(const char* file, int line, const char* format, ...);
    static void info(const char* file, int line, const char* format, ...);
    static void debug(const char* file, int line, const char* format, ...);
    static void trace(const char* file, int line, const char* format, ...);

private:
    static void logImpl(LogLevel level, const char* file, int line, 
                       const char* format, va_list args);
    static std::string formatMessage(LogLevel level, const char* file, int line, 
                                   const std::string& message);
    static const char* levelToString(LogLevel level);
    static const char* levelToColorCode(LogLevel level);
    
    static Config config_;
    static std::vector<LogOutput> custom_outputs_;
    static std::mutex log_mutex_;
    static std::ofstream log_file_;
};

} // namespace lua_runtime

// === 日志输出宏 ===
#if LUA_RUNTIME_ENABLE_LOGGING

    #define LUA_RUNTIME_LOG_ERROR(msg, ...) \
        do { if (LUA_RUNTIME_LOG_LEVEL >= 1) \
            lua_runtime::Logger::error(__FILE__, __LINE__, msg, ##__VA_ARGS__); } while(0)
    
    #define LUA_RUNTIME_LOG_WARN(msg, ...) \
        do { if (LUA_RUNTIME_LOG_LEVEL >= 2) \
            lua_runtime::Logger::warn(__FILE__, __LINE__, msg, ##__VA_ARGS__); } while(0)
    
    #define LUA_RUNTIME_LOG_INFO(msg, ...) \
        do { if (LUA_RUNTIME_LOG_LEVEL >= 3) \
            lua_runtime::Logger::info(__FILE__, __LINE__, msg, ##__VA_ARGS__); } while(0)
    
    #define LUA_RUNTIME_LOG_DEBUG(msg, ...) \
        do { if (LUA_RUNTIME_LOG_LEVEL >= 4) \
            lua_runtime::Logger::debug(__FILE__, __LINE__, msg, ##__VA_ARGS__); } while(0)
    
    #define LUA_RUNTIME_LOG_TRACE(msg, ...) \
        do { if (LUA_RUNTIME_LOG_LEVEL >= 5) \
            lua_runtime::Logger::trace(__FILE__, __LINE__, msg, ##__VA_ARGS__); } while(0)

#else
    // 编译时完全移除日志调用
    #define LUA_RUNTIME_LOG_ERROR(msg, ...)   ((void)0)
    #define LUA_RUNTIME_LOG_WARN(msg, ...)    ((void)0)
    #define LUA_RUNTIME_LOG_INFO(msg, ...)    ((void)0)
    #define LUA_RUNTIME_LOG_DEBUG(msg, ...)   ((void)0)
    #define LUA_RUNTIME_LOG_TRACE(msg, ...)   ((void)0)
#endif