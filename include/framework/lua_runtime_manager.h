/**
 * @file lua_runtime_manager.h
 * @brief Lua运行时管理器 - 核心API接口
 */

#pragma once

#include "memory_allocator.h"
#include "result.h"
#include "hot_reload.h"
#include "runtime_logger.h"

#include <sol/sol.hpp>
#include <memory>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

namespace lua_runtime {

/**
 * @brief Lua运行时管理器
 * 
 * 核心功能包括：
 * - Lua状态管理
 * - 绑定注册
 * - 脚本执行
 * - 热加载支持
 * - 内存分配器集成
 */
class LuaRuntimeManager : public HotReloadInterface {
public:
    // === 构造函数 ===
    
    /**
     * @brief 默认构造函数（使用标准分配器）
     */
    LuaRuntimeManager();
    
    /**
     * @brief 使用自定义分配器构造
     * @param allocator 自定义内存分配器
     */
    explicit LuaRuntimeManager(std::shared_ptr<MemoryAllocator> allocator);
    
    
    /**
     * @brief 析构函数
     */
    ~LuaRuntimeManager();
    
    // 禁止拷贝，允许移动
    LuaRuntimeManager(const LuaRuntimeManager&) = delete;
    LuaRuntimeManager& operator=(const LuaRuntimeManager&) = delete;
    LuaRuntimeManager(LuaRuntimeManager&&) noexcept;
    LuaRuntimeManager& operator=(LuaRuntimeManager&&) noexcept;
    
    // === 分配器管理 ===
    
    /**
     * @brief 获取当前分配器
     * @return 分配器引用
     */
    const MemoryAllocator& getAllocator() const;
    
    /**
     * @brief 替换分配器（危险操作，仅在特殊情况下使用）
     * @param new_allocator 新的分配器
     */
    void replaceAllocator(std::shared_ptr<MemoryAllocator> new_allocator);
    
    // === Lua状态访问 ===
    
    /**
     * @brief 获取Lua状态（可修改）
     * @return Lua状态引用
     */
    sol::state& getLuaState();
    
    /**
     * @brief 获取Lua状态（只读）
     * @return Lua状态常量引用
     */
    const sol::state& getLuaState() const;
    
    /**
     * @brief 检查Lua状态是否有效
     * @return 是否有效
     */
    bool isStateValid() const;
    
    /**
     * @brief 重置Lua状态
     */
    void resetState();
    
    // === 绑定注册 ===
    
    /**
     * @brief 注册绑定函数
     * @param binding_func 绑定函数，接受sol::state&参数
     * @return 注册结果
     */
    template<typename BindingFunc>
    Result<void> registerBindings(BindingFunc&& binding_func) {
        try {
            LUA_RUNTIME_LOG_DEBUG("注册绑定函数");
            binding_func(*lua_state_);
            onBindingsRegistered();
            LUA_RUNTIME_LOG_INFO("绑定函数注册成功");
            return makeSuccess();
        } catch (const std::exception& e) {
            LUA_RUNTIME_LOG_ERROR("绑定注册失败: %s", e.what());
            return makeError<void>(ErrorType::BINDING_REGISTRATION_FAILED, e.what());
        }
    }
    
    /**
     * @brief 批量注册多个绑定
     * @param funcs 绑定函数包
     * @return 注册结果
     */
    template<typename... BindingFuncs>
    Result<void> registerMultipleBindings(BindingFuncs&&... funcs) {
        std::vector<Result<void>> results;
        (results.push_back(registerBindings(std::forward<BindingFuncs>(funcs))), ...);
        
        for (const auto& result : results) {
            if (result.isError()) {
                return result;
            }
        }
        return makeSuccess();
    }
    
    // === 脚本执行 ===
    
    /**
     * @brief 执行Lua脚本
     * @param script 脚本内容
     * @return 执行结果
     */
    Result<sol::object> executeScript(const std::string& script);
    
    /**
     * @brief 执行Lua脚本文件
     * @param filename 文件名
     * @return 执行结果
     */
    Result<sol::object> executeFile(const std::string& filename);
    
    /**
     * @brief 调用Lua函数
     * @param func_name 函数名
     * @param args 参数包
     * @return 调用结果
     */
    template<typename... Args>
    Result<sol::object> callLuaFunction(const std::string& func_name, Args&&... args) {
        try {
            LUA_RUNTIME_LOG_DEBUG("调用Lua函数: %s", func_name.c_str());
            sol::function func = (*lua_state_)[func_name];
            if (!func.valid()) {
                return makeError<sol::object>(ErrorType::FUNCTION_NOT_FOUND, 
                                           "Function not found: " + func_name);
            }
            
            auto result = func(std::forward<Args>(args)...);
            if (!result.valid()) {
                sol::error err = result;
                return makeError<sol::object>(ErrorType::RUNTIME_ERROR, err.what(), func_name);
            }
            
            LUA_RUNTIME_LOG_DEBUG("Lua函数调用成功: %s", func_name.c_str());
            return makeSuccess(result.template get<sol::object>());
        } catch (const std::exception& e) {
            LUA_RUNTIME_LOG_ERROR("Lua函数调用失败 %s: %s", func_name.c_str(), e.what());
            return makeError<sol::object>(ErrorType::RUNTIME_ERROR, e.what(), func_name);
        }
    }
    
    // === HotReloadInterface实现 ===
    
    Result<void> registerHotReloadScript(const std::string& name, 
                                        const std::string& file_path) override;
    
    std::vector<HotReloadEvent> checkAndReloadScripts() override;
    
    HotReloadEvent reloadScript(const std::string& name) override;
    
    HotReloadEvent reloadScriptContent(const std::string& name, 
                                      const std::string& content) override;
    
    void addProtectedTable(const std::string& table_name) override;
    
    void removeProtectedTable(const std::string& table_name) override;
    
    std::vector<std::string> getProtectedTables() const override;
    
    void setPreReloadCallback(HotReloadCallback callback) override;
    
    void setPostReloadCallback(HotReloadCallback callback) override;
    
    std::vector<ScriptInfo> getRegisteredScripts() const override;
    
    bool needsReload(const std::string& name) const override;
    
    std::vector<HotReloadEvent> getReloadHistory() const override;

private:
    // === 私有成员 ===
    
    std::shared_ptr<MemoryAllocator> allocator_;
    std::unique_ptr<sol::state> lua_state_;
    
    // 热加载相关
    std::unordered_map<std::string, ScriptInfo> registered_scripts_;
    std::unordered_set<std::string> protected_tables_;
    HotReloadCallback pre_reload_callback_;
    HotReloadCallback post_reload_callback_;
    std::vector<HotReloadEvent> reload_history_;
    
    // === 私有方法 ===
    
    /**
     * @brief 初始化Lua状态并设置分配器
     */
    void initializeLuaWithAllocator();
    
    /**
     * @brief Lua分配器函数（C接口）
     */
    static void* luaAllocFunction(void* ud, void* ptr, size_t osize, size_t nsize);
    
    /**
     * @brief 绑定注册完成回调
     */
    void onBindingsRegistered();
    
    /**
     * @brief 检查文件是否已修改
     * @param script 脚本信息
     * @return 是否已修改
     */
    bool hasFileChanged(const ScriptInfo& script) const;
    
    /**
     * @brief 读取脚本文件内容
     * @param file_path 文件路径
     * @return 文件内容
     */
    Result<std::string> readScriptFile(const std::string& file_path) const;
    
    /**
     * @brief 执行热加载
     * @param name 脚本名称
     * @param content 脚本内容
     * @return 热加载事件
     */
    HotReloadEvent performReload(const std::string& name, const std::string& content);
    
    /**
     * @brief 备份保护的表
     */
    void backupProtectedTables();
    
    /**
     * @brief 恢复保护的表
     */
    void restoreProtectedTables();
    
    /**
     * @brief 计算内容哈希
     * @param content 内容
     * @return 哈希值
     */
    std::size_t calculateContentHash(const std::string& content) const;
    
    /**
     * @brief 获取文件修改时间
     * @param file_path 文件路径
     * @return 修改时间
     */
    std::time_t getFileModTime(const std::string& file_path) const;
    
    // 用于状态保护的临时存储
    std::unordered_map<std::string, sol::object> protected_table_backup_;
};

} // namespace lua_runtime