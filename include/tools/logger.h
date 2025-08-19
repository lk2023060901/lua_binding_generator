/**
 * @file logger.h
 * @brief 简单的日志封装，基于 spdlog
 * 
 * 提供基本的控制台和文件日志功能，专为 lua_binding_generator 设计
 */

#pragma once

#include <string>
#include <memory>

namespace spdlog {
    class logger;
}

namespace lua_binding_generator {

/**
 * @brief 简单的日志管理器
 * 
 * 提供基础的日志功能：
 * - 控制台输出（彩色）
 * - 可选的文件输出
 * - 不同级别的日志（info, success, error）
 */
class Logger {
public:
    /**
     * @brief 初始化日志系统
     */
    static void initialize();
    
    /**
     * @brief 输出信息日志
     * @param msg 日志消息
     */
    static void info(const std::string& msg);
    
    /**
     * @brief 输出成功日志（绿色）
     * @param msg 日志消息
     */
    static void success(const std::string& msg);
    
    /**
     * @brief 输出错误日志（红色）
     * @param msg 日志消息
     */
    static void error(const std::string& msg);
    
    /**
     * @brief 输出警告日志（黄色）
     * @param msg 日志消息
     */
    static void warning(const std::string& msg);
    
    /**
     * @brief 输出调试日志
     * @param msg 日志消息
     */
    static void debug(const std::string& msg);
    
    /**
     * @brief 设置日志文件路径
     * @param filename 日志文件名
     */
    static void setLogFile(const std::string& filename);
    
    /**
     * @brief 启用或禁用控制台输出
     * @param enable 是否启用控制台输出
     */
    static void enableConsole(bool enable = true);
    
    /**
     * @brief 启用或禁用详细输出模式
     * @param enable 是否启用详细模式
     */
    static void enableVerbose(bool enable = true);
    
    /**
     * @brief 刷新所有日志缓冲区
     */
    static void flush();

private:
    static std::shared_ptr<spdlog::logger> console_logger_;
    static std::shared_ptr<spdlog::logger> file_logger_;
    static bool console_enabled_;
    static bool verbose_enabled_;
    static bool initialized_;
    
    static void ensureInitialized();
};

} // namespace lua_binding_generator