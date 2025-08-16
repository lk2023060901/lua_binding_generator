/**
 * @file simple_main.cpp
 * @brief 简单示例的测试主程序
 */

#include "simple_example.h"
#include <sol/sol.hpp>
#include <iostream>
#include <filesystem>

// Forward declaration of binding function
void register_generated_module_bindings(sol::state& lua);

void runSimpleDemo() {
    std::cout << "=== Simple Example Demo ===" << std::endl;
    
    // 测试全局函数
    std::cout << "\n--- Global Functions ---" << std::endl;
    std::cout << "add(5, 3) = " << simple::add(5, 3) << std::endl;
    std::cout << "greet(\"World\") = " << simple::greet("World") << std::endl;
    std::cout << "isEven(42) = " << (simple::isEven(42) ? "true" : "false") << std::endl;
    
    // 测试枚举
    std::cout << "\n--- Enums ---" << std::endl;
    simple::Color color = simple::Color::RED;
    std::cout << "Color::RED value = " << static_cast<int>(color) << std::endl;
    
    // 测试常量
    std::cout << "\n--- Constants ---" << std::endl;
    std::cout << "MAX_COUNT = " << simple::MAX_COUNT << std::endl;
    std::cout << "VERSION = " << simple::VERSION << std::endl;
    std::cout << "APP_NAME = " << simple::APP_NAME << std::endl;
    
    // 测试Calculator类
    std::cout << "\n--- Calculator Class ---" << std::endl;
    simple::Calculator calc(10.0);
    std::cout << "Initial value: " << calc.getValue() << std::endl;
    std::cout << "After add(5): " << calc.add(5.0) << std::endl;
    std::cout << "After multiply(2): " << calc.multiply(2.0) << std::endl;
    std::cout << "After divide(3): " << calc.divide(3.0) << std::endl;
    
    std::cout << "Calculator::pi() = " << simple::Calculator::pi() << std::endl;
    std::cout << "Calculator::e() = " << simple::Calculator::e() << std::endl;
    
    // 测试DataContainer类
    std::cout << "\n--- DataContainer Class ---" << std::endl;
    simple::DataContainer container;
    container.setText("Test Data");
    container.addNumber(10);
    container.addNumber(20);
    container.addNumber(30);
    
    std::cout << "Text: " << container.getText() << std::endl;
    std::cout << "Number count: " << container.getNumberCount() << std::endl;
    std::cout << "Number at index 1: " << container.getNumberAt(1) << std::endl;
    std::cout << "Container: " << container.toString() << std::endl;
    
    // 测试StringUtils静态类
    std::cout << "\n--- StringUtils Static Class ---" << std::endl;
    std::string test_str = "Hello World";
    std::cout << "Original: " << test_str << std::endl;
    std::cout << "Upper: " << simple::StringUtils::toUpperCase(test_str) << std::endl;
    std::cout << "Lower: " << simple::StringUtils::toLowerCase(test_str) << std::endl;
    std::cout << "Reverse: " << simple::StringUtils::reverse(test_str) << std::endl;
    std::cout << "Length: " << simple::StringUtils::length(test_str) << std::endl;
    std::cout << "Contains 'World': " << (simple::StringUtils::contains(test_str, "World") ? "true" : "false") << std::endl;
    std::cout << "Starts with 'Hello': " << (simple::StringUtils::startsWith(test_str, "Hello") ? "true" : "false") << std::endl;
    
    // 测试字符串分割和连接
    std::vector<std::string> parts = simple::StringUtils::split("apple,banana,cherry", ',');
    std::cout << "Split result size: " << parts.size() << std::endl;
    std::string joined = simple::StringUtils::join(parts, " | ");
    std::cout << "Joined: " << joined << std::endl;
    
    std::cout << "\n=== Demo Completed ===" << std::endl;
}

void runLuaTests() {
    std::cout << "\n=== Running Lua Tests ===" << std::endl;
    
    // Initialize Lua state
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, 
                      sol::lib::math, sol::lib::table, sol::lib::io);
    
    try {
        // Register C++ bindings
        register_generated_module_bindings(lua);
        
        // Look for Lua test scripts in the scripts directory
        std::string scripts_dir = "examples/scripts";
        if (!std::filesystem::exists(scripts_dir)) {
            scripts_dir = "scripts"; // Try relative path
        }
        
        if (std::filesystem::exists(scripts_dir)) {
            std::cout << "Found scripts directory: " << scripts_dir << std::endl;
            
            std::string script_path = scripts_dir + "/test_simple.lua";
            if (std::filesystem::exists(script_path)) {
                std::cout << "\n--- Running test_simple.lua ---" << std::endl;
                lua.script_file(script_path);
            } else {
                std::cout << "⚠️  Test script not found: " << script_path << std::endl;
            }
        } else {
            std::cout << "⚠️  Scripts directory not found" << std::endl;
        }
        
    } catch (const sol::error& e) {
        std::cout << "❌ Lua error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << std::endl;
    }
    
    std::cout << "\n=== Lua Tests Complete ===" << std::endl;
}

int main() {
    try {
        runSimpleDemo();
        runLuaTests();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}