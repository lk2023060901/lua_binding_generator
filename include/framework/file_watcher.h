/**
 * @file file_watcher.h
 * @brief 文件监控器实现
 */

#pragma once

#include "hot_reload.h"
#include <thread>
#include <atomic>
#include <unordered_map>
#include <sys/stat.h>

namespace lua_runtime {

/**
 * @brief 基于轮询的文件监控器实现
 * 
 * 这是一个简单的文件监控器实现，适用于大多数平台。
 * 在生产环境中，建议使用平台特定的文件监控API（如inotify、kqueue、ReadDirectoryChangesW）
 */
class PollingFileWatcher : public FileWatcher {
private:
    /**
     * @brief 监控的文件信息
     */
    struct WatchedFile {
        std::string path;
        std::function<void(const std::string&)> callback;
        std::time_t last_modified;
        bool exists;
        
        WatchedFile(const std::string& file_path, 
                   std::function<void(const std::string&)> cb)
            : path(file_path), callback(cb), last_modified(0), exists(false) {
            updateStatus();
        }
        
        void updateStatus() {
            struct stat st;
            if (stat(path.c_str(), &st) == 0) {
                last_modified = st.st_mtime;
                exists = true;
            } else {
                exists = false;
            }
        }
        
        bool hasChanged() {
            std::time_t old_time = last_modified;
            bool old_exists = exists;
            
            updateStatus();
            
            // 检查文件是否有变化
            return (exists != old_exists) || 
                   (exists && last_modified != old_time);
        }
    };
    
    std::unordered_map<std::string, std::unique_ptr<WatchedFile>> watched_files_;
    std::thread polling_thread_;
    std::atomic<bool> running_{false};
    std::chrono::milliseconds poll_interval_;
    mutable std::mutex files_mutex_;

public:
    /**
     * @brief 构造函数
     * @param poll_interval_ms 轮询间隔（毫秒），默认500ms
     */
    explicit PollingFileWatcher(int poll_interval_ms = 500)
        : poll_interval_(poll_interval_ms) {}
    
    /**
     * @brief 析构函数
     */
    ~PollingFileWatcher() override {
        stop();
    }
    
    void watchFile(const std::string& file_path, 
                   std::function<void(const std::string&)> callback) override {
        std::lock_guard<std::mutex> lock(files_mutex_);
        
        auto watched_file = std::make_unique<WatchedFile>(file_path, callback);
        watched_files_[file_path] = std::move(watched_file);
        
        LUA_RUNTIME_LOG_DEBUG("开始监控文件: %s", file_path.c_str());
    }
    
    void unwatchFile(const std::string& file_path) override {
        std::lock_guard<std::mutex> lock(files_mutex_);
        
        auto it = watched_files_.find(file_path);
        if (it != watched_files_.end()) {
            watched_files_.erase(it);
            LUA_RUNTIME_LOG_DEBUG("停止监控文件: %s", file_path.c_str());
        }
    }
    
    void start() override {
        if (running_) {
            return;  // 已经在运行
        }
        
        running_ = true;
        polling_thread_ = std::thread([this]() {
            pollingLoop();
        });
        
        LUA_RUNTIME_LOG_INFO("文件监控器已启动，轮询间隔: %d ms", 
                           static_cast<int>(poll_interval_.count()));
    }
    
    void stop() override {
        if (!running_) {
            return;  // 已经停止
        }
        
        running_ = false;
        if (polling_thread_.joinable()) {
            polling_thread_.join();
        }
        
        LUA_RUNTIME_LOG_INFO("文件监控器已停止");
    }
    
    /**
     * @brief 设置轮询间隔
     * @param interval_ms 间隔毫秒数
     */
    void setPollInterval(int interval_ms) {
        poll_interval_ = std::chrono::milliseconds(interval_ms);
    }
    
    /**
     * @brief 获取当前监控的文件数量
     * @return 文件数量
     */
    size_t getWatchedFileCount() const {
        std::lock_guard<std::mutex> lock(files_mutex_);
        return watched_files_.size();
    }
    
    /**
     * @brief 手动检查所有文件（用于测试）
     */
    void checkAllFiles() {
        checkFiles();
    }

private:
    void pollingLoop() {
        LUA_RUNTIME_LOG_DEBUG("文件监控轮询循环开始");
        
        while (running_) {
            try {
                checkFiles();
            } catch (const std::exception& e) {
                LUA_RUNTIME_LOG_ERROR("文件监控轮询异常: %s", e.what());
            }
            
            // 可中断的睡眠
            auto start = std::chrono::steady_clock::now();
            while (running_ && 
                   (std::chrono::steady_clock::now() - start) < poll_interval_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        
        LUA_RUNTIME_LOG_DEBUG("文件监控轮询循环结束");
    }
    
    void checkFiles() {
        std::lock_guard<std::mutex> lock(files_mutex_);
        
        for (auto& [path, watched_file] : watched_files_) {
            try {
                if (watched_file->hasChanged()) {
                    LUA_RUNTIME_LOG_DEBUG("检测到文件变化: %s", path.c_str());
                    
                    // 调用回调函数
                    if (watched_file->callback) {
                        watched_file->callback(path);
                    }
                }
            } catch (const std::exception& e) {
                LUA_RUNTIME_LOG_ERROR("检查文件失败 %s: %s", path.c_str(), e.what());
            }
        }
    }
};

/**
 * @brief 文件监控器工厂
 * 
 * 提供创建不同类型文件监控器的工厂方法
 */
class FileWatcherFactory {
public:
    /**
     * @brief 创建轮询文件监控器
     * @param poll_interval_ms 轮询间隔（毫秒）
     * @return 文件监控器智能指针
     */
    static std::unique_ptr<FileWatcher> createPollingWatcher(int poll_interval_ms = 500) {
        return std::make_unique<PollingFileWatcher>(poll_interval_ms);
    }
    
    /**
     * @brief 创建默认文件监控器
     * @return 文件监控器智能指针
     */
    static std::unique_ptr<FileWatcher> createDefault() {
        return createPollingWatcher();
    }
    
    // 未来可以添加其他平台特定的监控器
    // static std::unique_ptr<FileWatcher> createINotifyWatcher();  // Linux
    // static std::unique_ptr<FileWatcher> createKQueueWatcher();   // macOS/FreeBSD
    // static std::unique_ptr<FileWatcher> createWin32Watcher();    // Windows
};

} // namespace lua_runtime