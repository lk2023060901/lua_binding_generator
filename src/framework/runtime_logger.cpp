/**
 * @file logger.cpp
 * @brief 日志系统实现
 */

#include "runtime_logger.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>

namespace lua_runtime {

// 静态成员初始化
Logger::Config Logger::config_;
std::vector<Logger::LogOutput> Logger::custom_outputs_;
std::mutex Logger::log_mutex_;
std::ofstream Logger::log_file_;

void Logger::configure(const Config& config) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    config_ = config;
    
    // 如果启用文件输出，打开日志文件
    if (config_.enable_file_output) {
        if (log_file_.is_open()) {
            log_file_.close();
        }
        log_file_.open(config_.log_file_path, std::ios::app);
    }
}

Logger::Config& Logger::getConfig() {
    return config_;
}

void Logger::setRuntimeLevel(LogLevel level) {
    config_.runtime_level = level;
}

LogLevel Logger::getRuntimeLevel() {
    return config_.runtime_level;
}

void Logger::addCustomOutput(LogOutput output) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    custom_outputs_.push_back(output);
}

void Logger::removeAllCustomOutputs() {
    std::lock_guard<std::mutex> lock(log_mutex_);
    custom_outputs_.clear();
}

void Logger::error(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logImpl(LogLevel::ERROR, file, line, format, args);
    va_end(args);
}

void Logger::warn(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logImpl(LogLevel::WARN, file, line, format, args);
    va_end(args);
}

void Logger::info(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logImpl(LogLevel::INFO, file, line, format, args);
    va_end(args);
}

void Logger::debug(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logImpl(LogLevel::DEBUG, file, line, format, args);
    va_end(args);
}

void Logger::trace(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logImpl(LogLevel::TRACE, file, line, format, args);
    va_end(args);
}

void Logger::logImpl(LogLevel level, const char* file, int line, 
                    const char* format, va_list args) {
    // 检查运行时级别
    if (level > config_.runtime_level) {
        return;
    }
    
    // 格式化消息
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    std::string message(buffer);
    
    // 格式化完整的日志条目
    std::string formatted_message = formatMessage(level, file, line, message);
    
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    // 控制台输出
    if (config_.enable_console_output) {
        if (config_.enable_color_output) {
            std::cout << levelToColorCode(level) << formatted_message << "\033[0m" << std::endl;
        } else {
            std::cout << formatted_message << std::endl;
        }
    }
    
    // 文件输出
    if (config_.enable_file_output && log_file_.is_open()) {
        log_file_ << formatted_message << std::endl;
        log_file_.flush();
    }
    
    // 自定义输出器
    for (const auto& output : custom_outputs_) {
        output(level, formatted_message);
    }
}

std::string Logger::formatMessage(LogLevel level, const char* file, int line, 
                                 const std::string& message) {
    std::ostringstream oss;
    
    // 时间戳
    if (config_.enable_timestamps) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        oss << "." << std::setfill('0') << std::setw(3) << ms.count() << " ";
    }
    
    // 线程ID
    if (config_.enable_thread_ids) {
        oss << "[" << std::this_thread::get_id() << "] ";
    }
    
    // 日志级别
    oss << "[" << levelToString(level) << "] ";
    
    // 消息内容
    oss << message;
    
    // 文件和行号
    if (file && line > 0) {
        // 只显示文件名，不显示完整路径
        const char* filename = strrchr(file, '/');
        if (!filename) {
            filename = strrchr(file, '\\');
        }
        filename = filename ? filename + 1 : file;
        
        oss << " (" << filename << ":" << line << ")";
    }
    
    return oss.str();
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::TRACE: return "TRACE";
        default: return "UNKNOWN";
    }
}

const char* Logger::levelToColorCode(LogLevel level) {
    switch (level) {
        case LogLevel::ERROR: return "\033[31m";  // 红色
        case LogLevel::WARN:  return "\033[33m";  // 黄色
        case LogLevel::INFO:  return "\033[32m";  // 绿色
        case LogLevel::DEBUG: return "\033[36m";  // 青色
        case LogLevel::TRACE: return "\033[37m";  // 白色
        default: return "\033[0m";                // 默认
    }
}

} // namespace lua_runtime