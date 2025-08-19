/**
 * @file hot_reload.h
 * @brief 脚本热加载功能接口
 */

#pragma once

#include "result.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <chrono>
#include <ctime>

namespace lua_runtime {

/**
 * @brief 脚本信息结构
 */
struct ScriptInfo {
    std::string name;           // 脚本名称
    std::string file_path;      // 文件路径
    std::string content;        // 脚本内容
    std::time_t last_modified;  // 最后修改时间
    std::size_t content_hash;   // 内容哈希
    
    ScriptInfo() = default;
    ScriptInfo(const std::string& name, const std::string& path)
        : name(name), file_path(path), last_modified(0), content_hash(0) {}
};

/**
 * @brief 热加载结果枚举
 */
enum class HotReloadResult {
    SUCCESS,          // 加载成功
    NO_CHANGES,       // 没有变化，跳过
    SYNTAX_ERROR,     // 语法错误
    RUNTIME_ERROR,    // 运行时错误
    FILE_NOT_FOUND,   // 文件未找到
    ROLLBACK_SUCCESS  // 回滚成功
};

/**
 * @brief 热加载事件结构
 */
struct HotReloadEvent {
    std::string script_name;
    HotReloadResult result;
    std::string error_message;
    std::chrono::steady_clock::time_point timestamp;
    
    HotReloadEvent(const std::string& name, HotReloadResult res, 
                   const std::string& error = "")
        : script_name(name), result(res), error_message(error)
        , timestamp(std::chrono::steady_clock::now()) {}
};

/**
 * @brief 热加载功能接口
 */
class HotReloadInterface {
public:
    /**
     * @brief 热加载回调函数类型
     */
    using HotReloadCallback = std::function<void(const HotReloadEvent&)>;
    
    virtual ~HotReloadInterface() = default;
    
    // === 热加载核心接口 ===
    
    /**
     * @brief 注册需要热加载的脚本
     * @param name 脚本名称
     * @param file_path 文件路径
     * @return 注册结果
     */
    virtual Result<void> registerHotReloadScript(const std::string& name, 
                                                const std::string& file_path) = 0;
    
    /**
     * @brief 手动触发热加载检查
     * @return 热加载事件列表
     */
    virtual std::vector<HotReloadEvent> checkAndReloadScripts() = 0;
    
    /**
     * @brief 重新加载特定脚本
     * @param name 脚本名称
     * @return 热加载事件
     */
    virtual HotReloadEvent reloadScript(const std::string& name) = 0;
    
    /**
     * @brief 重新加载脚本内容（直接传入内容）
     * @param name 脚本名称
     * @param content 脚本内容
     * @return 热加载事件
     */
    virtual HotReloadEvent reloadScriptContent(const std::string& name, 
                                              const std::string& content) = 0;
    
    // === 状态管理接口 ===
    
    /**
     * @brief 添加状态保护表（热加载时不清除这些表）
     * @param table_name 表名
     */
    virtual void addProtectedTable(const std::string& table_name) = 0;
    
    /**
     * @brief 移除状态保护表
     * @param table_name 表名
     */
    virtual void removeProtectedTable(const std::string& table_name) = 0;
    
    /**
     * @brief 获取保护的表列表
     * @return 保护表名列表
     */
    virtual std::vector<std::string> getProtectedTables() const = 0;
    
    /**
     * @brief 设置热加载前的回调
     * @param callback 回调函数
     */
    virtual void setPreReloadCallback(HotReloadCallback callback) = 0;
    
    /**
     * @brief 设置热加载后的回调
     * @param callback 回调函数
     */
    virtual void setPostReloadCallback(HotReloadCallback callback) = 0;
    
    // === 查询接口 ===
    
    /**
     * @brief 获取已注册的脚本列表
     * @return 脚本信息列表
     */
    virtual std::vector<ScriptInfo> getRegisteredScripts() const = 0;
    
    /**
     * @brief 检查脚本是否需要重新加载
     * @param name 脚本名称
     * @return 是否需要重新加载
     */
    virtual bool needsReload(const std::string& name) const = 0;
    
    /**
     * @brief 获取热加载历史
     * @return 热加载事件历史
     */
    virtual std::vector<HotReloadEvent> getReloadHistory() const = 0;
};

/**
 * @brief 文件监控器接口
 */
class FileWatcher {
public:
    virtual ~FileWatcher() = default;
    
    /**
     * @brief 添加监控文件
     * @param file_path 文件路径
     * @param callback 文件变化回调
     */
    virtual void watchFile(const std::string& file_path, 
                          std::function<void(const std::string&)> callback) = 0;
    
    /**
     * @brief 移除监控
     * @param file_path 文件路径
     */
    virtual void unwatchFile(const std::string& file_path) = 0;
    
    /**
     * @brief 启动监控
     */
    virtual void start() = 0;
    
    /**
     * @brief 停止监控
     */
    virtual void stop() = 0;
};

/**
 * @brief 热加载管理器（整合文件监控）
 */
class HotReloadManager {
private:
    HotReloadInterface& hot_reload_;
    std::unique_ptr<FileWatcher> file_watcher_;
    std::atomic<bool> auto_reload_enabled_{false};
    std::unordered_map<std::string, std::string> file_to_script_map_;
    
public:
    /**
     * @brief 构造函数
     * @param hot_reload 热加载接口实现
     */
    explicit HotReloadManager(HotReloadInterface& hot_reload)
        : hot_reload_(hot_reload) {}
    
    /**
     * @brief 设置文件监控器
     * @param watcher 文件监控器
     */
    void setFileWatcher(std::unique_ptr<FileWatcher> watcher) {
        file_watcher_ = std::move(watcher);
    }
    
    /**
     * @brief 启用/禁用自动热加载
     * @param enabled 是否启用
     */
    void enableAutoReload(bool enabled = true) {
        auto_reload_enabled_ = enabled;
        if (file_watcher_) {
            if (enabled) {
                file_watcher_->start();
            } else {
                file_watcher_->stop();
            }
        }
    }
    
    /**
     * @brief 检查是否启用自动热加载
     * @return 是否启用
     */
    bool isAutoReloadEnabled() const {
        return auto_reload_enabled_;
    }
    
    /**
     * @brief 注册脚本并开始监控
     * @param name 脚本名称
     * @param file_path 文件路径
     * @return 操作结果
     */
    Result<void> registerAndWatch(const std::string& name, 
                                 const std::string& file_path) {
        auto result = hot_reload_.registerHotReloadScript(name, file_path);
        if (result.isSuccess() && file_watcher_) {
            file_to_script_map_[file_path] = name;
            file_watcher_->watchFile(file_path, [this](const std::string& path) {
                onFileChanged(path);
            });
        }
        return result;
    }
    
    /**
     * @brief 停止监控特定脚本
     * @param name 脚本名称
     */
    void unregisterAndUnwatch(const std::string& name) {
        // 找到对应的文件路径
        for (auto it = file_to_script_map_.begin(); it != file_to_script_map_.end(); ++it) {
            if (it->second == name) {
                if (file_watcher_) {
                    file_watcher_->unwatchFile(it->first);
                }
                file_to_script_map_.erase(it);
                break;
            }
        }
    }
    
private:
    void onFileChanged(const std::string& file_path) {
        if (!auto_reload_enabled_) return;
        
        auto it = file_to_script_map_.find(file_path);
        if (it != file_to_script_map_.end()) {
            hot_reload_.reloadScript(it->second);
        }
    }
};

} // namespace lua_runtime