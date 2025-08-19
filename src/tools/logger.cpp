/**
 * @file logger.cpp
 * @brief 日志封装实现
 */

#include "logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <iostream>

namespace lua_binding_generator {

// 静态成员初始化
std::shared_ptr<spdlog::logger> Logger::console_logger_;
std::shared_ptr<spdlog::logger> Logger::file_logger_;
bool Logger::console_enabled_ = true;
bool Logger::verbose_enabled_ = false;
bool Logger::initialized_ = false;

void Logger::initialize() {
    if (initialized_) {
        return;
    }
    
    try {
        // 创建控制台日志器（带颜色）
        console_logger_ = spdlog::stdout_color_mt("console");
        console_logger_->set_pattern("[%^%l%$] %v");
        
        // 设置默认日志级别
        console_logger_->set_level(spdlog::level::info);
        
        initialized_ = true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize logger: " << e.what() << std::endl;
    }
}

void Logger::ensureInitialized() {
    if (!initialized_) {
        initialize();
    }
}

void Logger::info(const std::string& msg) {
    ensureInitialized();
    if (console_enabled_ && console_logger_) {
        console_logger_->info(msg);
    }
    if (file_logger_) {
        file_logger_->info(msg);
    }
}

void Logger::success(const std::string& msg) {
    ensureInitialized();
    if (console_enabled_ && console_logger_) {
        console_logger_->info("\033[32m✓\033[0m {}", msg);  // 绿色勾号
    }
    if (file_logger_) {
        file_logger_->info("[SUCCESS] {}", msg);
    }
}

void Logger::error(const std::string& msg) {
    ensureInitialized();
    if (console_enabled_ && console_logger_) {
        console_logger_->error(msg);
    }
    if (file_logger_) {
        file_logger_->error(msg);
    }
}

void Logger::warning(const std::string& msg) {
    ensureInitialized();
    if (console_enabled_ && console_logger_) {
        console_logger_->warn(msg);
    }
    if (file_logger_) {
        file_logger_->warn(msg);
    }
}

void Logger::debug(const std::string& msg) {
    ensureInitialized();
    if (verbose_enabled_) {
        if (console_enabled_ && console_logger_) {
            console_logger_->debug(msg);
        }
        if (file_logger_) {
            file_logger_->debug(msg);
        }
    }
}

void Logger::setLogFile(const std::string& filename) {
    ensureInitialized();
    try {
        file_logger_ = spdlog::basic_logger_mt("file", filename);
        file_logger_->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
        file_logger_->set_level(spdlog::level::debug);  // 文件记录所有级别
        
        // 日志文件设置成功，无需输出消息
    } catch (const std::exception& e) {
        error("Failed to set log file '" + filename + "': " + e.what());
    }
}

void Logger::enableConsole(bool enable) {
    if (!enable && console_enabled_) {
        // 在禁用控制台之前输出最后一条消息
        if (console_logger_) {
            console_logger_->info("Console output disabled");
        }
    }
    console_enabled_ = enable;
}

void Logger::enableVerbose(bool enable) {
    verbose_enabled_ = enable;
    ensureInitialized();
    
    if (enable) {
        if (console_logger_) {
            console_logger_->set_level(spdlog::level::debug);
        }
        info("Verbose mode enabled");
    } else {
        if (console_logger_) {
            console_logger_->set_level(spdlog::level::info);
        }
    }
}

void Logger::flush() {
    if (console_logger_) {
        console_logger_->flush();
    }
    if (file_logger_) {
        file_logger_->flush();
    }
}

} // namespace lua_binding_generator