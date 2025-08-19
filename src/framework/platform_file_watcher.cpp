/**
 * @file platform_file_watcher.cpp
 * @brief 平台特定的高效文件监控器实现
 */

#include "platform_file_watcher.h"
#include "file_watcher.h"

namespace lua_runtime {

std::unique_ptr<FileWatcher> EnhancedFileWatcherFactory::createPollingWatcher(int poll_interval_ms) {
    return std::make_unique<PollingFileWatcher>(poll_interval_ms);
}

} // namespace lua_runtime