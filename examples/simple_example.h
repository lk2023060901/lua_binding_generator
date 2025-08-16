/**
 * @file simple_example.h
 * @brief 简单的 Lua 绑定生成器使用示例
 * 
 * 这是一个最基础的示例，展示如何使用零配置的导出宏
 * 适合初学者理解和学习
 */

#pragma once

#include "../include/export_macros.h"
#include <string>
#include <vector>

// 模块定义
EXPORT_LUA_MODULE(SimpleExample)

namespace simple {

/**
 * @brief 简单枚举示例
 */
enum class EXPORT_LUA_ENUM() Color {
    RED,
    GREEN,
    BLUE,
    YELLOW
};

/**
 * @brief 简单常量示例
 */
EXPORT_LUA_CONSTANT()
static const int MAX_COUNT = 10;

EXPORT_LUA_CONSTANT()
static const double VERSION = 1.0;

EXPORT_LUA_CONSTANT()
static const std::string APP_NAME = "Simple Example";

/**
 * @brief 简单函数示例
 */
EXPORT_LUA_FUNCTION()
int add(int a, int b);

EXPORT_LUA_FUNCTION()
std::string greet(const std::string& name);

EXPORT_LUA_FUNCTION()
bool isEven(int number);

/**
 * @brief 简单类示例 - 自动导出所有公共成员
 */
class EXPORT_LUA_CLASS() Calculator {
public:
    Calculator();
    Calculator(double initial_value);
    
    // 属性 - 自动推导为 Lua 属性
    double getValue() const;
    void setValue(double value);
    
    // 方法 - 自动导出
    double add(double x);
    double subtract(double x);
    double multiply(double x);
    double divide(double x);
    
    void reset();
    void clear();
    
    // 静态方法
    static double pi();
    static double e();

private:
    double value_;
};

/**
 * @brief 简单的数据容器类
 */
class EXPORT_LUA_CLASS() DataContainer {
public:
    DataContainer();
    
    // 字符串操作
    std::string getText() const;
    void setText(const std::string& text);
    
    // 数字列表操作
    std::vector<int> getNumbers() const;
    void addNumber(int number);
    void clearNumbers();
    
    // 查询方法
    int getNumberCount() const;
    int getNumberAt(int index) const;
    
    // 实用方法
    std::string toString() const;

private:
    std::string text_;
    std::vector<int> numbers_;
};

/**
 * @brief 静态工具类示例
 */
class EXPORT_LUA_STATIC_CLASS() StringUtils {
public:
    // 字符串操作
    static std::string toUpperCase(const std::string& str);
    static std::string toLowerCase(const std::string& str);
    static std::string reverse(const std::string& str);
    
    // 字符串查询
    static int length(const std::string& str);
    static bool contains(const std::string& str, const std::string& substr);
    static bool startsWith(const std::string& str, const std::string& prefix);
    static bool endsWith(const std::string& str, const std::string& suffix);
    
    // 字符串转换
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string join(const std::vector<std::string>& parts, const std::string& separator);
};

} // namespace simple