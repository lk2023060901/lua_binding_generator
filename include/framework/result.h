/**
 * @file result.h
 * @brief Result类型和错误处理系统
 */

#pragma once

#include <variant>
#include <string>
#include <vector>
#include <chrono>
#include <stdexcept>
#include <sstream>

namespace lua_runtime {

/**
 * @brief 错误类型枚举
 */
enum class ErrorType {
    // 脚本相关
    SYNTAX_ERROR,
    COMPILE_ERROR,
    RUNTIME_ERROR,
    SCRIPT_LOAD_ERROR,
    
    // 函数相关
    FUNCTION_NOT_FOUND,
    FUNCTION_EXISTS,
    FUNCTION_REGISTER_ERROR,
    INVALID_ARGUMENTS,
    
    // 绑定相关
    BINDING_REGISTRATION_FAILED,
    BINDING_NOT_FOUND,
    
    // 热加载相关
    HOT_RELOAD_FAILED,
    FILE_NOT_FOUND,
    FILE_READ_ERROR,
    
    // 安全相关
    SECURITY_ERROR,
    PERMISSION_DENIED,
    EXECUTION_TIMEOUT,
    MEMORY_LIMIT_EXCEEDED,
    
    // 系统相关
    INTERNAL_ERROR,
    INVALID_STATE,
    RESOURCE_EXHAUSTED
};

/**
 * @brief 错误信息结构
 */
struct ErrorInfo {
    ErrorType type;
    std::string message;
    std::string context;  // 额外的上下文信息
    int line_number = 0;
    std::chrono::steady_clock::time_point timestamp;
    std::vector<std::string> stack_trace;
    
    ErrorInfo(ErrorType t, const std::string& msg, 
             const std::string& ctx = "", int line = 0)
        : type(t), message(msg), context(ctx), line_number(line)
        , timestamp(std::chrono::steady_clock::now()) {}
    
    /**
     * @brief 转换为字符串表示
     */
    std::string toString() const {
        std::stringstream ss;
        ss << "[" << errorTypeToString(type) << "] ";
        ss << message;
        if (!context.empty()) {
            ss << " (in " << context;
            if (line_number > 0) {
                ss << ":" << line_number;
            }
            ss << ")";
        }
        return ss.str();
    }
    
private:
    static const char* errorTypeToString(ErrorType type) {
        switch (type) {
            case ErrorType::SYNTAX_ERROR: return "SYNTAX_ERROR";
            case ErrorType::COMPILE_ERROR: return "COMPILE_ERROR";
            case ErrorType::RUNTIME_ERROR: return "RUNTIME_ERROR";
            case ErrorType::SCRIPT_LOAD_ERROR: return "SCRIPT_LOAD_ERROR";
            case ErrorType::FUNCTION_NOT_FOUND: return "FUNCTION_NOT_FOUND";
            case ErrorType::FUNCTION_EXISTS: return "FUNCTION_EXISTS";
            case ErrorType::FUNCTION_REGISTER_ERROR: return "FUNCTION_REGISTER_ERROR";
            case ErrorType::INVALID_ARGUMENTS: return "INVALID_ARGUMENTS";
            case ErrorType::BINDING_REGISTRATION_FAILED: return "BINDING_REGISTRATION_FAILED";
            case ErrorType::BINDING_NOT_FOUND: return "BINDING_NOT_FOUND";
            case ErrorType::HOT_RELOAD_FAILED: return "HOT_RELOAD_FAILED";
            case ErrorType::FILE_NOT_FOUND: return "FILE_NOT_FOUND";
            case ErrorType::FILE_READ_ERROR: return "FILE_READ_ERROR";
            case ErrorType::SECURITY_ERROR: return "SECURITY_ERROR";
            case ErrorType::PERMISSION_DENIED: return "PERMISSION_DENIED";
            case ErrorType::EXECUTION_TIMEOUT: return "EXECUTION_TIMEOUT";
            case ErrorType::MEMORY_LIMIT_EXCEEDED: return "MEMORY_LIMIT_EXCEEDED";
            case ErrorType::INTERNAL_ERROR: return "INTERNAL_ERROR";
            case ErrorType::INVALID_STATE: return "INVALID_STATE";
            case ErrorType::RESOURCE_EXHAUSTED: return "RESOURCE_EXHAUSTED";
            default: return "UNKNOWN_ERROR";
        }
    }
};

/**
 * @brief Result模板类 - 用于表示可能失败的操作结果
 */
template<typename T>
class Result {
private:
    std::variant<T, ErrorInfo> data_;
    
public:
    Result(T&& value) : data_(std::move(value)) {}
    Result(const T& value) : data_(value) {}
    Result(const ErrorInfo& error) : data_(error) {}
    Result(ErrorInfo&& error) : data_(std::move(error)) {}
    
    /**
     * @brief 检查是否成功
     */
    bool isSuccess() const { return std::holds_alternative<T>(data_); }
    
    /**
     * @brief 检查是否有错误
     */
    bool isError() const { return std::holds_alternative<ErrorInfo>(data_); }
    
    /**
     * @brief 获取成功值（如果失败会抛出异常）
     */
    const T& value() const { 
        if (!isSuccess()) {
            throw std::runtime_error("Result contains error: " + error().toString());
        }
        return std::get<T>(data_); 
    }
    
    /**
     * @brief 获取成功值（如果失败会抛出异常）
     */
    T& value() { 
        if (!isSuccess()) {
            throw std::runtime_error("Result contains error: " + error().toString());
        }
        return std::get<T>(data_); 
    }
    
    /**
     * @brief 获取错误信息（如果成功会抛出异常）
     */
    const ErrorInfo& error() const {
        if (!isError()) {
            throw std::runtime_error("Result contains value, not error");
        }
        return std::get<ErrorInfo>(data_);
    }
    
    /**
     * @brief 获取值或默认值
     */
    T valueOr(T&& default_value) const {
        return isSuccess() ? value() : std::move(default_value);
    }
    
    /**
     * @brief 映射操作（如果成功则应用函数，否则传递错误）
     */
    template<typename Func>
    auto map(Func&& func) const -> Result<decltype(func(value()))> {
        using ReturnType = decltype(func(value()));
        if (isSuccess()) {
            return Result<ReturnType>(func(value()));
        }
        return Result<ReturnType>(error());
    }
};

/**
 * @brief Result<void>的特化
 */
template<>
class Result<void> {
private:
    std::optional<ErrorInfo> error_;
    
public:
    Result() = default;
    Result(const ErrorInfo& error) : error_(error) {}
    Result(ErrorInfo&& error) : error_(std::move(error)) {}
    
    bool isSuccess() const { return !error_.has_value(); }
    bool isError() const { return error_.has_value(); }
    
    const ErrorInfo& error() const { 
        if (!isError()) {
            throw std::runtime_error("Result contains success, not error");
        }
        return *error_; 
    }
    
    void value() const {
        if (isError()) {
            throw std::runtime_error("Result contains error: " + error().toString());
        }
    }
};

/**
 * @brief 便利函数：创建成功的Result
 */
template<typename T>
Result<T> makeSuccess(T&& value) {
    return Result<T>(std::forward<T>(value));
}

/**
 * @brief 便利函数：创建void成功的Result
 */
inline Result<void> makeSuccess() {
    return Result<void>();
}

/**
 * @brief 便利函数：创建错误的Result
 */
template<typename T>
Result<T> makeError(ErrorType type, const std::string& message, 
                   const std::string& context = "", int line = 0) {
    return Result<T>(ErrorInfo(type, message, context, line));
}

/**
 * @brief 便利函数：创建void错误的Result
 */
inline Result<void> makeError(ErrorType type, const std::string& message, 
                             const std::string& context = "", int line = 0) {
    return Result<void>(ErrorInfo(type, message, context, line));
}

} // namespace lua_runtime