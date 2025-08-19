/**
 * @file lua_runtime_manager.cpp
 * @brief Lua运行时管理器实现
 */

#include "lua_runtime_manager.h"
#include <fstream>
#include <sys/stat.h>
#include <functional>

namespace lua_runtime {

// === 构造函数和析构函数 ===

LuaRuntimeManager::LuaRuntimeManager() 
    : allocator_(std::make_shared<DefaultAllocator>()) {
    LUA_RUNTIME_LOG_INFO("创建LuaRuntimeManager，使用默认分配器");
    initializeLuaWithAllocator();
}

LuaRuntimeManager::LuaRuntimeManager(std::shared_ptr<MemoryAllocator> allocator)
    : allocator_(allocator) {
    LUA_RUNTIME_LOG_INFO("创建LuaRuntimeManager，使用共享分配器");
    initializeLuaWithAllocator();
}

LuaRuntimeManager::~LuaRuntimeManager() {
    LUA_RUNTIME_LOG_INFO("销毁LuaRuntimeManager");
    lua_state_.reset();
}

LuaRuntimeManager::LuaRuntimeManager(LuaRuntimeManager&& other) noexcept
    : allocator_(other.allocator_)
    , lua_state_(std::move(other.lua_state_))
    , registered_scripts_(std::move(other.registered_scripts_))
    , protected_tables_(std::move(other.protected_tables_))
    , pre_reload_callback_(std::move(other.pre_reload_callback_))
    , post_reload_callback_(std::move(other.post_reload_callback_))
    , reload_history_(std::move(other.reload_history_))
    , protected_table_backup_(std::move(other.protected_table_backup_)) {
    LUA_RUNTIME_LOG_DEBUG("LuaRuntimeManager移动构造");
}

LuaRuntimeManager& LuaRuntimeManager::operator=(LuaRuntimeManager&& other) noexcept {
    if (this != &other) {
        allocator_ = other.allocator_;
        lua_state_ = std::move(other.lua_state_);
        registered_scripts_ = std::move(other.registered_scripts_);
        protected_tables_ = std::move(other.protected_tables_);
        pre_reload_callback_ = std::move(other.pre_reload_callback_);
        post_reload_callback_ = std::move(other.post_reload_callback_);
        reload_history_ = std::move(other.reload_history_);
        protected_table_backup_ = std::move(other.protected_table_backup_);
        LUA_RUNTIME_LOG_DEBUG("LuaRuntimeManager移动赋值");
    }
    return *this;
}

// === 分配器管理 ===

const MemoryAllocator& LuaRuntimeManager::getAllocator() const {
    return *allocator_;
}

void LuaRuntimeManager::replaceAllocator(std::shared_ptr<MemoryAllocator> new_allocator) {
    LUA_RUNTIME_LOG_WARN("替换内存分配器（危险操作）");
    allocator_ = new_allocator;
    // 注意：这里不重新初始化Lua状态，因为可能导致数据丢失
}

// === Lua状态访问 ===

sol::state& LuaRuntimeManager::getLuaState() {
    return *lua_state_;
}

const sol::state& LuaRuntimeManager::getLuaState() const {
    return *lua_state_;
}

bool LuaRuntimeManager::isStateValid() const {
    return lua_state_ && lua_state_->lua_state();
}

void LuaRuntimeManager::resetState() {
    LUA_RUNTIME_LOG_INFO("重置Lua状态");
    lua_state_.reset();
    initializeLuaWithAllocator();
}

// === 脚本执行 ===

Result<sol::object> LuaRuntimeManager::executeScript(const std::string& script) {
    if (!isStateValid()) {
        return makeError<sol::object>(ErrorType::INVALID_STATE, "Lua状态无效");
    }
    
    try {
        LUA_RUNTIME_LOG_DEBUG("执行Lua脚本，长度: %zu", script.length());
        auto result = lua_state_->safe_script(script);
        
        if (!result.valid()) {
            sol::error err = result;
            LUA_RUNTIME_LOG_ERROR("脚本执行失败: %s", err.what());
            return makeError<sol::object>(ErrorType::RUNTIME_ERROR, err.what());
        }
        
        LUA_RUNTIME_LOG_DEBUG("脚本执行成功");
        return makeSuccess(result.get<sol::object>());
    } catch (const std::exception& e) {
        LUA_RUNTIME_LOG_ERROR("脚本执行异常: %s", e.what());
        return makeError<sol::object>(ErrorType::RUNTIME_ERROR, e.what());
    }
}

Result<sol::object> LuaRuntimeManager::executeFile(const std::string& filename) {
    LUA_RUNTIME_LOG_DEBUG("执行Lua文件: %s", filename.c_str());
    
    auto content_result = readScriptFile(filename);
    if (content_result.isError()) {
        return makeError<sol::object>(content_result.error().type, 
                                    content_result.error().message, filename);
    }
    
    return executeScript(content_result.value());
}

// === 热加载实现 ===

Result<void> LuaRuntimeManager::registerHotReloadScript(const std::string& name, 
                                                       const std::string& file_path) {
    LUA_RUNTIME_LOG_INFO("注册热加载脚本: %s -> %s", name.c_str(), file_path.c_str());
    
    // 检查文件是否存在
    std::ifstream file(file_path);
    if (!file.good()) {
        return makeError(ErrorType::FILE_NOT_FOUND, "文件不存在: " + file_path);
    }
    
    ScriptInfo script_info(name, file_path);
    script_info.last_modified = getFileModTime(file_path);
    
    // 读取初始内容
    auto content_result = readScriptFile(file_path);
    if (content_result.isError()) {
        return makeError(content_result.error().type, content_result.error().message);
    }
    
    script_info.content = content_result.value();
    script_info.content_hash = calculateContentHash(script_info.content);
    
    registered_scripts_[name] = std::move(script_info);
    
    LUA_RUNTIME_LOG_INFO("热加载脚本注册成功: %s", name.c_str());
    return makeSuccess();
}

std::vector<HotReloadEvent> LuaRuntimeManager::checkAndReloadScripts() {
    std::vector<HotReloadEvent> events;
    
    for (auto& [name, script] : registered_scripts_) {
        if (hasFileChanged(script)) {
            LUA_RUNTIME_LOG_DEBUG("检测到文件变化: %s", script.file_path.c_str());
            auto event = reloadScript(name);
            events.push_back(event);
        }
    }
    
    return events;
}

HotReloadEvent LuaRuntimeManager::reloadScript(const std::string& name) {
    LUA_RUNTIME_LOG_INFO("重新加载脚本: %s", name.c_str());
    
    auto it = registered_scripts_.find(name);
    if (it == registered_scripts_.end()) {
        LUA_RUNTIME_LOG_ERROR("脚本未注册: %s", name.c_str());
        return HotReloadEvent(name, HotReloadResult::FILE_NOT_FOUND, "脚本未注册");
    }
    
    auto content_result = readScriptFile(it->second.file_path);
    if (content_result.isError()) {
        LUA_RUNTIME_LOG_ERROR("读取脚本文件失败: %s", content_result.error().message.c_str());
        return HotReloadEvent(name, HotReloadResult::FILE_NOT_FOUND, 
                            content_result.error().message);
    }
    
    // 检查内容是否真的有变化
    std::size_t new_hash = calculateContentHash(content_result.value());
    if (new_hash == it->second.content_hash) {
        LUA_RUNTIME_LOG_DEBUG("脚本内容无变化: %s", name.c_str());
        return HotReloadEvent(name, HotReloadResult::NO_CHANGES);
    }
    
    // 更新脚本信息
    it->second.content = content_result.value();
    it->second.content_hash = new_hash;
    it->second.last_modified = getFileModTime(it->second.file_path);
    
    return performReload(name, content_result.value());
}

HotReloadEvent LuaRuntimeManager::reloadScriptContent(const std::string& name, 
                                                     const std::string& content) {
    LUA_RUNTIME_LOG_INFO("重新加载脚本内容: %s", name.c_str());
    
    auto it = registered_scripts_.find(name);
    if (it != registered_scripts_.end()) {
        // 更新内容
        it->second.content = content;
        it->second.content_hash = calculateContentHash(content);
    } else {
        // 创建新的脚本信息
        ScriptInfo script_info(name, "");
        script_info.content = content;
        script_info.content_hash = calculateContentHash(content);
        registered_scripts_[name] = std::move(script_info);
    }
    
    return performReload(name, content);
}

void LuaRuntimeManager::addProtectedTable(const std::string& table_name) {
    protected_tables_.insert(table_name);
    LUA_RUNTIME_LOG_DEBUG("添加保护表: %s", table_name.c_str());
}

void LuaRuntimeManager::removeProtectedTable(const std::string& table_name) {
    protected_tables_.erase(table_name);
    LUA_RUNTIME_LOG_DEBUG("移除保护表: %s", table_name.c_str());
}

std::vector<std::string> LuaRuntimeManager::getProtectedTables() const {
    return std::vector<std::string>(protected_tables_.begin(), protected_tables_.end());
}

void LuaRuntimeManager::setPreReloadCallback(HotReloadCallback callback) {
    pre_reload_callback_ = callback;
}

void LuaRuntimeManager::setPostReloadCallback(HotReloadCallback callback) {
    post_reload_callback_ = callback;
}

std::vector<ScriptInfo> LuaRuntimeManager::getRegisteredScripts() const {
    std::vector<ScriptInfo> scripts;
    for (const auto& [name, script] : registered_scripts_) {
        scripts.push_back(script);
    }
    return scripts;
}

bool LuaRuntimeManager::needsReload(const std::string& name) const {
    auto it = registered_scripts_.find(name);
    if (it == registered_scripts_.end()) {
        return false;
    }
    return hasFileChanged(it->second);
}

std::vector<HotReloadEvent> LuaRuntimeManager::getReloadHistory() const {
    return reload_history_;
}

// === 私有方法实现 ===

void LuaRuntimeManager::initializeLuaWithAllocator() {
    LUA_RUNTIME_LOG_DEBUG("初始化Lua状态和分配器");
    
    // 检查是否使用默认分配器
    if (std::dynamic_pointer_cast<DefaultAllocator>(allocator_)) {
        // 使用默认分配器，直接创建标准Sol2状态
        lua_state_ = std::make_unique<sol::state>();
    } else {
        // 使用自定义分配器，通过Sol2的分配器接口
        lua_state_ = std::make_unique<sol::state>(
            &sol::default_at_panic,
            &LuaRuntimeManager::luaAllocFunction,
            allocator_.get()
        );
    }
    
    // 打开标准库
    lua_state_->open_libraries(
        sol::lib::base,
        sol::lib::package,
        sol::lib::coroutine,
        sol::lib::string,
        sol::lib::os,
        sol::lib::math,
        sol::lib::table,
        sol::lib::debug,
        sol::lib::bit32,
        sol::lib::io,
        sol::lib::utf8
    );
    
    LUA_RUNTIME_LOG_DEBUG("Lua状态初始化完成");
}

void* LuaRuntimeManager::luaAllocFunction(void* ud, void* ptr, size_t osize, size_t nsize) {
    MemoryAllocator* allocator = static_cast<MemoryAllocator*>(ud);
    
    if (nsize == 0) {
        // 释放内存
        if (ptr) {
            allocator->deallocate(ptr, osize);
        }
        return nullptr;
    } else if (ptr == nullptr) {
        // 分配新内存
        return allocator->allocate(nsize);
    } else {
        // 重新分配内存
        return allocator->reallocate(ptr, osize, nsize);
    }
}

void LuaRuntimeManager::onBindingsRegistered() {
    LUA_RUNTIME_LOG_DEBUG("绑定注册完成回调");
    // 可以在这里执行一些初始化后的操作
}

bool LuaRuntimeManager::hasFileChanged(const ScriptInfo& script) const {
    if (script.file_path.empty()) {
        return false;  // 没有文件路径，无法检查
    }
    
    std::time_t current_time = getFileModTime(script.file_path);
    return current_time != script.last_modified;
}

Result<std::string> LuaRuntimeManager::readScriptFile(const std::string& file_path) const {
    std::ifstream file(file_path);
    if (!file.good()) {
        return makeError<std::string>(ErrorType::FILE_NOT_FOUND, 
                                    "无法打开文件: " + file_path);
    }
    
    try {
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return makeSuccess(std::move(content));
    } catch (const std::exception& e) {
        return makeError<std::string>(ErrorType::FILE_READ_ERROR, 
                                    "读取文件失败: " + std::string(e.what()));
    }
}

HotReloadEvent LuaRuntimeManager::performReload(const std::string& name, 
                                               const std::string& content) {
    LUA_RUNTIME_LOG_DEBUG("执行热加载: %s", name.c_str());
    
    // 创建事件对象
    HotReloadEvent event(name, HotReloadResult::SUCCESS);
    
    // 执行前回调
    if (pre_reload_callback_) {
        pre_reload_callback_(event);
    }
    
    try {
        // 备份保护的表
        backupProtectedTables();
        
        // 执行脚本
        auto result = executeScript(content);
        if (result.isError()) {
            event.result = HotReloadResult::RUNTIME_ERROR;
            event.error_message = result.error().message;
            
            // 恢复保护的表
            restoreProtectedTables();
        } else {
            // 恢复保护的表（合并数据）
            restoreProtectedTables();
            LUA_RUNTIME_LOG_INFO("脚本热加载成功: %s", name.c_str());
        }
        
    } catch (const std::exception& e) {
        event.result = HotReloadResult::RUNTIME_ERROR;
        event.error_message = e.what();
        LUA_RUNTIME_LOG_ERROR("热加载异常: %s", e.what());
        
        // 恢复保护的表
        restoreProtectedTables();
    }
    
    // 执行后回调
    if (post_reload_callback_) {
        post_reload_callback_(event);
    }
    
    // 记录到历史
    reload_history_.push_back(event);
    
    return event;
}

void LuaRuntimeManager::backupProtectedTables() {
    protected_table_backup_.clear();
    
    for (const auto& table_name : protected_tables_) {
        try {
            sol::object table_obj = (*lua_state_)[table_name];
            if (table_obj.valid()) {
                protected_table_backup_[table_name] = table_obj;
                LUA_RUNTIME_LOG_TRACE("备份保护表: %s", table_name.c_str());
            }
        } catch (const std::exception& e) {
            LUA_RUNTIME_LOG_WARN("备份保护表失败 %s: %s", table_name.c_str(), e.what());
        }
    }
}

void LuaRuntimeManager::restoreProtectedTables() {
    for (const auto& [table_name, table_obj] : protected_table_backup_) {
        try {
            (*lua_state_)[table_name] = table_obj;
            LUA_RUNTIME_LOG_TRACE("恢复保护表: %s", table_name.c_str());
        } catch (const std::exception& e) {
            LUA_RUNTIME_LOG_WARN("恢复保护表失败 %s: %s", table_name.c_str(), e.what());
        }
    }
    
    protected_table_backup_.clear();
}

std::size_t LuaRuntimeManager::calculateContentHash(const std::string& content) const {
    return std::hash<std::string>{}(content);
}

std::time_t LuaRuntimeManager::getFileModTime(const std::string& file_path) const {
    struct stat st;
    if (stat(file_path.c_str(), &st) == 0) {
        return st.st_mtime;
    }
    return 0;
}

} // namespace lua_runtime