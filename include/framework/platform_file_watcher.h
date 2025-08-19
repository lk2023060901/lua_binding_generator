/**
 * @file platform_file_watcher.h
 * @brief 平台特定的高效文件监控器实现
 */

#pragma once

#include "hot_reload.h"
#include "runtime_logger.h"
#include <thread>
#include <atomic>
#include <unordered_map>
#include <queue>

#ifdef __APPLE__
#include <sys/event.h>
#include <unistd.h>
#include <fcntl.h>
#elif __linux__
#include <sys/inotify.h>
#include <unistd.h>
#elif _WIN32
#include <windows.h>
#include <filesystem>
#endif

namespace lua_runtime {

#ifdef __APPLE__
/**
 * @brief macOS/FreeBSD kqueue 文件监控器
 * 
 * 使用 kqueue 实现高效的文件系统事件监控
 */
class KQueueFileWatcher : public FileWatcher {
private:
    struct WatchedFile {
        std::string path;
        std::function<void(const std::string&)> callback;
        int fd;
        
        WatchedFile(const std::string& file_path, 
                   std::function<void(const std::string&)> cb)
            : path(file_path), callback(cb), fd(-1) {}
    };
    
    int kqueue_fd_;
    std::unordered_map<std::string, std::unique_ptr<WatchedFile>> watched_files_;
    std::unordered_map<int, std::string> fd_to_path_;
    std::thread monitor_thread_;
    std::atomic<bool> running_{false};
    mutable std::mutex files_mutex_;

public:
    KQueueFileWatcher() : kqueue_fd_(-1) {
        kqueue_fd_ = kqueue();
        if (kqueue_fd_ == -1) {
            throw std::runtime_error("Failed to create kqueue");
        }
    }
    
    ~KQueueFileWatcher() override {
        stop();
        if (kqueue_fd_ != -1) {
            close(kqueue_fd_);
        }
    }
    
    void watchFile(const std::string& file_path, 
                   std::function<void(const std::string&)> callback) override {
        std::lock_guard<std::mutex> lock(files_mutex_);
        
        auto watched_file = std::make_unique<WatchedFile>(file_path, callback);
        
        // 打开文件描述符
        watched_file->fd = open(file_path.c_str(), O_RDONLY);
        if (watched_file->fd == -1) {
            LUA_RUNTIME_LOG_ERROR("无法打开文件监控: %s", file_path.c_str());
            return;
        }
        
        // 创建 kevent 结构
        struct kevent event;
        EV_SET(&event, watched_file->fd, EVFILT_VNODE, 
               EV_ADD | EV_CLEAR, NOTE_WRITE | NOTE_DELETE | NOTE_RENAME, 0, nullptr);
        
        if (kevent(kqueue_fd_, &event, 1, nullptr, 0, nullptr) == -1) {
            close(watched_file->fd);
            LUA_RUNTIME_LOG_ERROR("无法添加 kqueue 事件: %s", file_path.c_str());
            return;
        }
        
        fd_to_path_[watched_file->fd] = file_path;
        watched_files_[file_path] = std::move(watched_file);
        
        LUA_RUNTIME_LOG_DEBUG("kqueue 开始监控文件: %s", file_path.c_str());
    }
    
    void unwatchFile(const std::string& file_path) override {
        std::lock_guard<std::mutex> lock(files_mutex_);
        
        auto it = watched_files_.find(file_path);
        if (it != watched_files_.end()) {
            // 移除 kqueue 事件
            struct kevent event;
            EV_SET(&event, it->second->fd, EVFILT_VNODE, EV_DELETE, 0, 0, nullptr);
            kevent(kqueue_fd_, &event, 1, nullptr, 0, nullptr);
            
            // 清理资源
            fd_to_path_.erase(it->second->fd);
            close(it->second->fd);
            watched_files_.erase(it);
            
            LUA_RUNTIME_LOG_DEBUG("kqueue 停止监控文件: %s", file_path.c_str());
        }
    }
    
    void start() override {
        if (running_) {
            return;
        }
        
        running_ = true;
        monitor_thread_ = std::thread([this]() {
            monitorLoop();
        });
        
        LUA_RUNTIME_LOG_INFO("kqueue 文件监控器已启动");
    }
    
    void stop() override {
        if (!running_) {
            return;
        }
        
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
        
        LUA_RUNTIME_LOG_INFO("kqueue 文件监控器已停止");
    }

private:
    void monitorLoop() {
        const int max_events = 64;
        struct kevent events[max_events];
        
        while (running_) {
            // 设置超时，定期检查 running_ 状态
            struct timespec timeout = {1, 0}; // 1秒超时
            
            int event_count = kevent(kqueue_fd_, nullptr, 0, events, max_events, &timeout);
            
            if (event_count == -1) {
                if (errno == EINTR) {
                    continue;
                }
                LUA_RUNTIME_LOG_ERROR("kqueue 监控错误: %s", strerror(errno));
                break;
            }
            
            for (int i = 0; i < event_count; ++i) {
                handleEvent(events[i]);
            }
        }
    }
    
    void handleEvent(const struct kevent& event) {
        std::lock_guard<std::mutex> lock(files_mutex_);
        
        auto fd_it = fd_to_path_.find(static_cast<int>(event.ident));
        if (fd_it == fd_to_path_.end()) {
            return;
        }
        
        const std::string& file_path = fd_it->second;
        auto file_it = watched_files_.find(file_path);
        if (file_it == watched_files_.end()) {
            return;
        }
        
        LUA_RUNTIME_LOG_DEBUG("kqueue 检测到文件变化: %s", file_path.c_str());
        
        if (file_it->second->callback) {
            file_it->second->callback(file_path);
        }
    }
};

#elif __linux__

/**
 * @brief Linux inotify 文件监控器
 * 
 * 使用 inotify 实现高效的文件系统事件监控
 */
class INotifyFileWatcher : public FileWatcher {
private:
    struct WatchedFile {
        std::string path;
        std::function<void(const std::string&)> callback;
        int watch_descriptor;
        
        WatchedFile(const std::string& file_path, 
                   std::function<void(const std::string&)> cb)
            : path(file_path), callback(cb), watch_descriptor(-1) {}
    };
    
    int inotify_fd_;
    std::unordered_map<std::string, std::unique_ptr<WatchedFile>> watched_files_;
    std::unordered_map<int, std::string> wd_to_path_;
    std::thread monitor_thread_;
    std::atomic<bool> running_{false};
    mutable std::mutex files_mutex_;

public:
    INotifyFileWatcher() : inotify_fd_(-1) {
        inotify_fd_ = inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
        if (inotify_fd_ == -1) {
            throw std::runtime_error("Failed to initialize inotify");
        }
    }
    
    ~INotifyFileWatcher() override {
        stop();
        if (inotify_fd_ != -1) {
            close(inotify_fd_);
        }
    }
    
    void watchFile(const std::string& file_path, 
                   std::function<void(const std::string&)> callback) override {
        std::lock_guard<std::mutex> lock(files_mutex_);
        
        auto watched_file = std::make_unique<WatchedFile>(file_path, callback);
        
        // 添加 inotify 监控
        int wd = inotify_add_watch(inotify_fd_, file_path.c_str(), 
                                  IN_MODIFY | IN_DELETE_SELF | IN_MOVE_SELF);
        if (wd == -1) {
            LUA_RUNTIME_LOG_ERROR("无法添加 inotify 监控: %s", file_path.c_str());
            return;
        }
        
        watched_file->watch_descriptor = wd;
        wd_to_path_[wd] = file_path;
        watched_files_[file_path] = std::move(watched_file);
        
        LUA_RUNTIME_LOG_DEBUG("inotify 开始监控文件: %s", file_path.c_str());
    }
    
    void unwatchFile(const std::string& file_path) override {
        std::lock_guard<std::mutex> lock(files_mutex_);
        
        auto it = watched_files_.find(file_path);
        if (it != watched_files_.end()) {
            // 移除 inotify 监控
            inotify_rm_watch(inotify_fd_, it->second->watch_descriptor);
            
            // 清理资源
            wd_to_path_.erase(it->second->watch_descriptor);
            watched_files_.erase(it);
            
            LUA_RUNTIME_LOG_DEBUG("inotify 停止监控文件: %s", file_path.c_str());
        }
    }
    
    void start() override {
        if (running_) {
            return;
        }
        
        running_ = true;
        monitor_thread_ = std::thread([this]() {
            monitorLoop();
        });
        
        LUA_RUNTIME_LOG_INFO("inotify 文件监控器已启动");
    }
    
    void stop() override {
        if (!running_) {
            return;
        }
        
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
        
        LUA_RUNTIME_LOG_INFO("inotify 文件监控器已停止");
    }

private:
    void monitorLoop() {
        const size_t buffer_size = 4096;
        char buffer[buffer_size];
        
        while (running_) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(inotify_fd_, &read_fds);
            
            // 设置超时，定期检查 running_ 状态
            struct timeval timeout = {1, 0}; // 1秒超时
            
            int select_result = select(inotify_fd_ + 1, &read_fds, nullptr, nullptr, &timeout);
            
            if (select_result == -1) {
                if (errno == EINTR) {
                    continue;
                }
                LUA_RUNTIME_LOG_ERROR("inotify select 错误: %s", strerror(errno));
                break;
            }
            
            if (select_result == 0) {
                continue; // 超时，继续循环
            }
            
            ssize_t bytes_read = read(inotify_fd_, buffer, buffer_size);
            if (bytes_read <= 0) {
                continue;
            }
            
            // 处理事件
            for (size_t i = 0; i < static_cast<size_t>(bytes_read);) {
                struct inotify_event* event = reinterpret_cast<struct inotify_event*>(&buffer[i]);
                handleEvent(*event);
                i += sizeof(struct inotify_event) + event->len;
            }
        }
    }
    
    void handleEvent(const struct inotify_event& event) {
        std::lock_guard<std::mutex> lock(files_mutex_);
        
        auto wd_it = wd_to_path_.find(event.wd);
        if (wd_it == wd_to_path_.end()) {
            return;
        }
        
        const std::string& file_path = wd_it->second;
        auto file_it = watched_files_.find(file_path);
        if (file_it == watched_files_.end()) {
            return;
        }
        
        LUA_RUNTIME_LOG_DEBUG("inotify 检测到文件变化: %s", file_path.c_str());
        
        if (file_it->second->callback) {
            file_it->second->callback(file_path);
        }
    }
};

#elif _WIN32

/**
 * @brief Windows ReadDirectoryChangesW 文件监控器
 * 
 * 使用 Windows API 实现高效的文件系统事件监控
 */
class Win32FileWatcher : public FileWatcher {
private:
    struct WatchedFile {
        std::string path;
        std::string directory;
        std::string filename;
        std::function<void(const std::string&)> callback;
        HANDLE dir_handle;
        
        WatchedFile(const std::string& file_path, 
                   std::function<void(const std::string&)> cb)
            : path(file_path), callback(cb), dir_handle(INVALID_HANDLE_VALUE) {
            
            std::filesystem::path p(file_path);
            directory = p.parent_path().string();
            filename = p.filename().string();
        }
    };
    
    std::unordered_map<std::string, std::unique_ptr<WatchedFile>> watched_files_;
    std::unordered_map<HANDLE, std::string> handle_to_path_;
    std::thread monitor_thread_;
    std::atomic<bool> running_{false};
    mutable std::mutex files_mutex_;

public:
    Win32FileWatcher() = default;
    
    ~Win32FileWatcher() override {
        stop();
    }
    
    void watchFile(const std::string& file_path, 
                   std::function<void(const std::string&)> callback) override {
        std::lock_guard<std::mutex> lock(files_mutex_);
        
        auto watched_file = std::make_unique<WatchedFile>(file_path, callback);
        
        // 打开目录句柄
        watched_file->dir_handle = CreateFileA(
            watched_file->directory.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            nullptr
        );
        
        if (watched_file->dir_handle == INVALID_HANDLE_VALUE) {
            LUA_RUNTIME_LOG_ERROR("无法打开目录监控: %s", watched_file->directory.c_str());
            return;
        }
        
        handle_to_path_[watched_file->dir_handle] = file_path;
        watched_files_[file_path] = std::move(watched_file);
        
        LUA_RUNTIME_LOG_DEBUG("Win32 开始监控文件: %s", file_path.c_str());
    }
    
    void unwatchFile(const std::string& file_path) override {
        std::lock_guard<std::mutex> lock(files_mutex_);
        
        auto it = watched_files_.find(file_path);
        if (it != watched_files_.end()) {
            // 关闭目录句柄
            if (it->second->dir_handle != INVALID_HANDLE_VALUE) {
                CloseHandle(it->second->dir_handle);
                handle_to_path_.erase(it->second->dir_handle);
            }
            watched_files_.erase(it);
            
            LUA_RUNTIME_LOG_DEBUG("Win32 停止监控文件: %s", file_path.c_str());
        }
    }
    
    void start() override {
        if (running_) {
            return;
        }
        
        running_ = true;
        monitor_thread_ = std::thread([this]() {
            monitorLoop();
        });
        
        LUA_RUNTIME_LOG_INFO("Win32 文件监控器已启动");
    }
    
    void stop() override {
        if (!running_) {
            return;
        }
        
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
        
        LUA_RUNTIME_LOG_INFO("Win32 文件监控器已停止");
    }

private:
    void monitorLoop() {
        const DWORD buffer_size = 4096;
        char buffer[buffer_size];
        
        while (running_) {
            std::vector<HANDLE> handles;
            std::vector<std::string> paths;
            
            {
                std::lock_guard<std::mutex> lock(files_mutex_);
                for (const auto& [path, watched_file] : watched_files_) {
                    if (watched_file->dir_handle != INVALID_HANDLE_VALUE) {
                        handles.push_back(watched_file->dir_handle);
                        paths.push_back(path);
                    }
                }
            }
            
            if (handles.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            
            DWORD wait_result = WaitForMultipleObjects(
                static_cast<DWORD>(handles.size()),
                handles.data(),
                FALSE,
                1000 // 1秒超时
            );
            
            if (wait_result >= WAIT_OBJECT_0 && 
                wait_result < WAIT_OBJECT_0 + handles.size()) {
                
                size_t index = wait_result - WAIT_OBJECT_0;
                handleDirectoryChange(handles[index], paths[index]);
            }
        }
    }
    
    void handleDirectoryChange(HANDLE dir_handle, const std::string& file_path) {
        std::lock_guard<std::mutex> lock(files_mutex_);
        
        auto it = watched_files_.find(file_path);
        if (it == watched_files_.end()) {
            return;
        }
        
        LUA_RUNTIME_LOG_DEBUG("Win32 检测到文件变化: %s", file_path.c_str());
        
        if (it->second->callback) {
            it->second->callback(file_path);
        }
    }
};

#endif

/**
 * @brief 增强的文件监控器工厂
 */
class EnhancedFileWatcherFactory {
public:
    /**
     * @brief 创建最优的文件监控器
     * @return 文件监控器智能指针
     */
    static std::unique_ptr<FileWatcher> createOptimal() {
#ifdef __APPLE__
        try {
            return std::make_unique<KQueueFileWatcher>();
        } catch (const std::exception& e) {
            LUA_RUNTIME_LOG_WARN("kqueue 创建失败，回退到轮询: %s", e.what());
            return createPollingWatcher();
        }
#elif __linux__
        try {
            return std::make_unique<INotifyFileWatcher>();
        } catch (const std::exception& e) {
            LUA_RUNTIME_LOG_WARN("inotify 创建失败，回退到轮询: %s", e.what());
            return createPollingWatcher();
        }
#elif _WIN32
        try {
            return std::make_unique<Win32FileWatcher>();
        } catch (const std::exception& e) {
            LUA_RUNTIME_LOG_WARN("Win32 监控器创建失败，回退到轮询: %s", e.what());
            return createPollingWatcher();
        }
#else
        return createPollingWatcher();
#endif
    }
    
    /**
     * @brief 创建轮询文件监控器（兜底方案）
     */
    static std::unique_ptr<FileWatcher> createPollingWatcher(int poll_interval_ms = 500);
};

} // namespace lua_runtime