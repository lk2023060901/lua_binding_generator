/**
 * @file lua_binding_runtime.h
 * @brief Lua绑定运行时库 - 主头文件
 * 
 * 这个头文件包含了使用Lua绑定运行时库所需的所有核心接口。
 * 该库专为配合lua_binding_generator工具生成的绑定代码而设计。
 */

#pragma once

// 核心组件
#include "lua_runtime_manager.h"
#include "memory_allocator.h"
#include "advanced_allocators.h"
#include "result.h"
#include "runtime_logger.h"
#include "hot_reload.h"
#include "file_watcher.h"
#include "platform_file_watcher.h"

/**
 * @brief Lua绑定运行时库命名空间
 * 
 * 包含所有运行时功能，包括：
 * - LuaRuntimeManager: 核心运行时管理器
 * - MemoryAllocator: 内存分配器接口
 * - HotReloadInterface: 脚本热加载功能
 * - Logger: 日志系统
 * - Result: 错误处理类型
 */
namespace lua_runtime {

/**
 * @brief 版本信息
 */
struct Version {
    static constexpr int MAJOR = 1;
    static constexpr int MINOR = 0;
    static constexpr int PATCH = 0;
    static constexpr const char* STRING = "1.0.0";
};

/**
 * @brief 获取库版本字符串
 * @return 版本字符串
 */
inline const char* getVersionString() {
    return Version::STRING;
}

/**
 * @brief 检查是否启用日志功能
 * @return 是否启用日志
 */
inline bool isLoggingEnabled() {
    #if LUA_RUNTIME_ENABLE_LOGGING
        return true;
    #else
        return false;
    #endif
}

/**
 * @brief 获取编译时日志级别
 * @return 日志级别
 */
inline int getCompiletimeLogLevel() {
    return LUA_RUNTIME_LOG_LEVEL;
}

} // namespace lua_runtime

/**
 * @brief 使用示例
 * 
 * @code
 * #include "lua_binding_runtime.h"
 * #include "generated_bindings.h"  // 由lua_binding_generator生成
 * 
 * int main() {
 *     using namespace lua_runtime;
 *     
 *     // 创建运行时管理器
 *     LuaRuntimeManager runtime;
 *     
 *     // 注册生成的绑定
 *     auto result = runtime.registerBindings(register_generated_module_bindings);
 *     if (result.isError()) {
 *         std::cerr << "绑定注册失败: " << result.error().toString() << std::endl;
 *         return 1;
 *     }
 *     
 *     // 注册热加载脚本
 *     runtime.registerHotReloadScript("main_script", "scripts/main.lua");
 *     
 *     // 执行脚本
 *     auto script_result = runtime.executeScript("print('Hello, Lua Runtime!')");
 *     if (script_result.isError()) {
 *         std::cerr << "脚本执行失败: " << script_result.error().toString() << std::endl;
 *         return 1;
 *     }
 *     
 *     // 游戏主循环
 *     while (true) {
 *         // 检查热加载
 *         runtime.checkAndReloadScripts();
 *         
 *         // 执行游戏逻辑
 *         runtime.callLuaFunction("update", 1.0/60.0);  // 60 FPS
 *         
 *         std::this_thread::sleep_for(std::chrono::milliseconds(16));
 *     }
 *     
 *     return 0;
 * }
 * @endcode
 */