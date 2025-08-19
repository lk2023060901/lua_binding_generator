# API 参考文档

## 目录

- [LuaRuntimeManager](#luaruntimemanager)
- [MemoryAllocator](#memoryallocator)
- [HotReload系统](#hotreload系统)
- [Logger](#logger)
- [Result和错误处理](#result和错误处理)
- [FileWatcher](#filewatcher)

## LuaRuntimeManager

核心运行时管理器类，提供Lua状态管理、脚本执行和热加载功能。

### 构造函数

```cpp
// 使用默认内存分配器
LuaRuntimeManager();

// 使用自定义内存分配器（unique_ptr）
LuaRuntimeManager(std::unique_ptr<MemoryAllocator> allocator);

// 使用共享内存分配器（shared_ptr）
LuaRuntimeManager(std::shared_ptr<MemoryAllocator> allocator);

// 移动构造函数
LuaRuntimeManager(LuaRuntimeManager&& other) noexcept;
```

### 内存分配器管理

```cpp
// 获取当前分配器的引用
const MemoryAllocator& getAllocator() const;

// 替换分配器（危险操作，不会重新初始化Lua状态）
void replaceAllocator(std::unique_ptr<MemoryAllocator> new_allocator);
```

### Lua状态访问

```cpp
// 获取Sol2状态引用
sol::state& getLuaState();
const sol::state& getLuaState() const;

// 检查状态是否有效
bool isStateValid() const;

// 重置Lua状态（清除所有数据）
void resetState();
```

### 绑定注册

```cpp
// 注册C++绑定到Lua
template<typename BindingFunc>
Result<void> registerBindings(BindingFunc&& binding_func);

// 示例使用
auto result = runtime.registerBindings([](sol::state& lua) {
    lua["myFunction"] = &my_cpp_function;
    lua.new_usertype<MyClass>("MyClass",
        sol::constructors<MyClass()>(),
        "method", &MyClass::method
    );
});
```

### 脚本执行

```cpp
// 执行Lua脚本字符串
Result<sol::object> executeScript(const std::string& script);

// 执行Lua文件
Result<sol::object> executeFile(const std::string& filename);

// 调用Lua函数
template<typename ReturnType, typename... Args>
Result<ReturnType> callLuaFunction(const std::string& function_name, Args&&... args);

// 示例使用
auto result = runtime.executeScript("print('Hello, World!')");
if (result.isError()) {
    std::cerr << "错误: " << result.error().message << std::endl;
}

auto func_result = runtime.callLuaFunction<float>("calculateDistance", 0, 0, 3, 4);
if (func_result.isSuccess()) {
    float distance = func_result.value();
}
```

### 热加载功能

```cpp
// 注册热加载脚本
Result<void> registerHotReloadScript(const std::string& name, const std::string& file_path);

// 检查并重载所有已注册的脚本
std::vector<HotReloadEvent> checkAndReloadScripts();

// 手动重载指定脚本
HotReloadEvent reloadScript(const std::string& name);

// 重载脚本内容（不从文件读取）
HotReloadEvent reloadScriptContent(const std::string& name, const std::string& content);

// 状态保护
void addProtectedTable(const std::string& table_name);
void removeProtectedTable(const std::string& table_name);
std::vector<std::string> getProtectedTables() const;

// 热加载回调
void setPreReloadCallback(HotReloadCallback callback);
void setPostReloadCallback(HotReloadCallback callback);

// 查询功能
std::vector<ScriptInfo> getRegisteredScripts() const;
bool needsReload(const std::string& name) const;
std::vector<HotReloadEvent> getReloadHistory() const;
```

### 使用示例

```cpp
// 基础使用
LuaRuntimeManager runtime;
runtime.registerBindings(my_bindings_function);

// 热加载设置
runtime.registerHotReloadScript("config", "config.lua");
runtime.addProtectedTable("GameData");

runtime.setPostReloadCallback([](const HotReloadEvent& event) {
    if (event.result == HotReloadResult::SUCCESS) {
        std::cout << "脚本重载成功: " << event.script_name << std::endl;
    }
});

// 主循环
while (running) {
    runtime.checkAndReloadScripts();
    runtime.callLuaFunction("update", delta_time);
}
```

## MemoryAllocator

内存分配器接口，允许自定义Lua的内存管理策略。

### 接口定义

```cpp
class MemoryAllocator {
public:
    virtual ~MemoryAllocator() = default;
    
    // 分配内存
    virtual void* allocate(size_t size, size_t alignment = sizeof(void*)) = 0;
    
    // 释放内存
    virtual void deallocate(void* ptr, size_t size) = 0;
    
    // 重新分配内存
    virtual void* reallocate(void* ptr, size_t old_size, size_t new_size) = 0;
};
```

### 默认实现

```cpp
class DefaultAllocator : public MemoryAllocator {
public:
    void* allocate(size_t size, size_t alignment = sizeof(void*)) override {
        return std::aligned_alloc(alignment, size);
    }
    
    void deallocate(void* ptr, size_t size) override {
        std::free(ptr);
    }
    
    void* reallocate(void* ptr, size_t old_size, size_t new_size) override {
        return std::realloc(ptr, new_size);
    }
};
```

### 自定义分配器示例

```cpp
class PoolAllocator : public MemoryAllocator {
private:
    void* pool_memory_;
    size_t pool_size_;
    std::mutex mutex_;
    
public:
    PoolAllocator(size_t pool_size) : pool_size_(pool_size) {
        pool_memory_ = std::malloc(pool_size_);
        // 初始化内存池...
    }
    
    void* allocate(size_t size, size_t alignment) override {
        std::lock_guard<std::mutex> lock(mutex_);
        // 从内存池分配...
    }
    
    // ... 其他方法
};

// 使用自定义分配器
auto allocator = std::make_unique<PoolAllocator>(1024 * 1024);  // 1MB池
LuaRuntimeManager runtime(std::move(allocator));
```

## HotReload系统

脚本热加载系统，支持运行时动态重载Lua脚本。

### 核心类型

```cpp
// 热加载结果枚举
enum class HotReloadResult {
    SUCCESS,        // 重载成功
    NO_CHANGES,     // 文件无变化
    FILE_NOT_FOUND, // 文件未找到
    RUNTIME_ERROR   // 运行时错误
};

// 热加载事件
struct HotReloadEvent {
    std::string script_name;
    HotReloadResult result;
    std::string error_message;
    std::chrono::system_clock::time_point timestamp;
    
    HotReloadEvent(const std::string& name, HotReloadResult res, 
                   const std::string& error = "");
};

// 脚本信息
struct ScriptInfo {
    std::string name;
    std::string file_path;
    std::string content;
    std::size_t content_hash;
    std::time_t last_modified;
    
    ScriptInfo(const std::string& script_name, const std::string& path);
};

// 热加载回调函数类型
using HotReloadCallback = std::function<void(const HotReloadEvent&)>;
```

### HotReloadInterface

```cpp
class HotReloadInterface {
public:
    virtual ~HotReloadInterface() = default;
    
    // 注册热加载脚本
    virtual Result<void> registerHotReloadScript(const std::string& name, 
                                                 const std::string& file_path) = 0;
    
    // 检查并重载脚本
    virtual std::vector<HotReloadEvent> checkAndReloadScripts() = 0;
    
    // 手动重载脚本
    virtual HotReloadEvent reloadScript(const std::string& name) = 0;
    
    // 重载脚本内容
    virtual HotReloadEvent reloadScriptContent(const std::string& name, 
                                              const std::string& content) = 0;
    
    // 状态保护
    virtual void addProtectedTable(const std::string& table_name) = 0;
    virtual void removeProtectedTable(const std::string& table_name) = 0;
    virtual std::vector<std::string> getProtectedTables() const = 0;
    
    // 回调设置
    virtual void setPreReloadCallback(HotReloadCallback callback) = 0;
    virtual void setPostReloadCallback(HotReloadCallback callback) = 0;
    
    // 查询功能
    virtual std::vector<ScriptInfo> getRegisteredScripts() const = 0;
    virtual bool needsReload(const std::string& name) const = 0;
    virtual std::vector<HotReloadEvent> getReloadHistory() const = 0;
};
```

### 使用示例

```cpp
// 注册需要热加载的脚本
runtime.registerHotReloadScript("game_logic", "scripts/game_logic.lua");
runtime.registerHotReloadScript("ai_behavior", "scripts/ai.lua");

// 保护重要数据不被重载覆盖
runtime.addProtectedTable("PlayerData");
runtime.addProtectedTable("GameState");

// 设置重载回调
runtime.setPreReloadCallback([](const HotReloadEvent& event) {
    std::cout << "准备重载: " << event.script_name << std::endl;
});

runtime.setPostReloadCallback([](const HotReloadEvent& event) {
    switch (event.result) {
        case HotReloadResult::SUCCESS:
            std::cout << "重载成功: " << event.script_name << std::endl;
            break;
        case HotReloadResult::RUNTIME_ERROR:
            std::cerr << "重载失败: " << event.error_message << std::endl;
            break;
        // ... 处理其他情况
    }
});

// 在主循环中检查热加载
while (game_running) {
    auto events = runtime.checkAndReloadScripts();
    // 处理重载事件...
    
    // 游戏逻辑
    runtime.callLuaFunction("update", delta_time);
}
```

## Logger

编译时控制的日志系统，支持多级别日志和自定义输出。

### 日志级别

```cpp
enum class LogLevel {
    ERROR = 1,
    WARN  = 2,
    INFO  = 3,
    DEBUG = 4,
    TRACE = 5
};
```

### 配置结构

```cpp
struct Config {
    LogLevel runtime_level = LogLevel::INFO;
    bool enable_console_output = true;
    bool enable_file_output = false;
    bool enable_color_output = true;
    bool enable_timestamps = true;
    bool enable_thread_ids = false;
    std::string log_file_path = "lua_runtime.log";
};
```

### 日志宏

```cpp
// 编译时控制的日志宏
LUA_RUNTIME_LOG_ERROR("错误信息: %s", error_msg);
LUA_RUNTIME_LOG_WARN("警告信息: %d", warning_code);
LUA_RUNTIME_LOG_INFO("信息: %s", info);
LUA_RUNTIME_LOG_DEBUG("调试: %d, %s", value, debug_info);
LUA_RUNTIME_LOG_TRACE("跟踪: %p", pointer);
```

### 静态方法

```cpp
class Logger {
public:
    // 配置日志系统
    static void configure(const Config& config);
    static Config& getConfig();
    
    // 运行时级别控制
    static void setRuntimeLevel(LogLevel level);
    static LogLevel getRuntimeLevel();
    
    // 自定义输出器
    using LogOutput = std::function<void(LogLevel, const std::string&)>;
    static void addCustomOutput(LogOutput output);
    static void removeAllCustomOutputs();
    
    // 日志函数（通常通过宏调用）
    static void error(const char* file, int line, const char* format, ...);
    static void warn(const char* file, int line, const char* format, ...);
    static void info(const char* file, int line, const char* format, ...);
    static void debug(const char* file, int line, const char* format, ...);
    static void trace(const char* file, int line, const char* format, ...);
};
```

### 使用示例

```cpp
// 配置日志系统
Logger::Config config;
config.runtime_level = LogLevel::DEBUG;
config.enable_console_output = true;
config.enable_file_output = true;
config.log_file_path = "game.log";
config.enable_color_output = true;
config.enable_timestamps = true;
Logger::configure(config);

// 添加自定义输出器
Logger::addCustomOutput([](LogLevel level, const std::string& message) {
    // 发送到远程日志服务器
    sendToRemoteLogger(level, message);
});

// 使用日志
LUA_RUNTIME_LOG_INFO("游戏开始，玩家数: %d", player_count);
LUA_RUNTIME_LOG_DEBUG("加载配置文件: %s", config_file.c_str());

// 运行时调整日志级别
Logger::setRuntimeLevel(LogLevel::WARN);  // 只显示警告和错误
```

## Result和错误处理

类型安全的错误处理系统，避免异常的性能开销。

### Result类型

```cpp
template<typename T>
class Result {
public:
    // 检查结果
    bool isSuccess() const;
    bool isError() const;
    
    // 获取值（仅在成功时有效）
    const T& value() const;
    T& value();
    
    // 获取错误信息（仅在失败时有效）
    const ErrorInfo& error() const;
    
    // 操作符重载
    explicit operator bool() const { return isSuccess(); }
};

// 特化版本，用于void返回类型
template<>
class Result<void> {
public:
    bool isSuccess() const;
    bool isError() const;
    const ErrorInfo& error() const;
    explicit operator bool() const { return isSuccess(); }
};
```

### 错误信息

```cpp
enum class ErrorType {
    NONE = 0,
    INVALID_ARGUMENT,
    INVALID_STATE,
    RUNTIME_ERROR,
    FILE_NOT_FOUND,
    FILE_READ_ERROR,
    MEMORY_ERROR,
    LUA_ERROR
};

struct ErrorInfo {
    ErrorType type;
    std::string message;
    std::string context;
    
    ErrorInfo(ErrorType t, const std::string& msg, const std::string& ctx = "");
    std::string toString() const;
};
```

### 辅助函数

```cpp
// 创建成功结果
template<typename T>
Result<T> makeSuccess(T&& value);

Result<void> makeSuccess();

// 创建错误结果
template<typename T>
Result<T> makeError(ErrorType type, const std::string& message, 
                    const std::string& context = "");
```

### 使用示例

```cpp
// 返回Result的函数
Result<int> divideNumbers(int a, int b) {
    if (b == 0) {
        return makeError<int>(ErrorType::INVALID_ARGUMENT, "除数不能为零");
    }
    return makeSuccess(a / b);
}

// 使用Result
auto result = divideNumbers(10, 2);
if (result.isSuccess()) {
    std::cout << "结果: " << result.value() << std::endl;
} else {
    std::cerr << "错误: " << result.error().toString() << std::endl;
}

// 链式调用
auto script_result = runtime.executeScript("return 42");
if (script_result) {  // 简化的成功检查
    auto lua_value = script_result.value();
    // 处理Lua返回值...
}
```

## FileWatcher

文件监控系统，用于检测脚本文件变化并触发热加载。

### FileWatcher接口

```cpp
class FileWatcher {
public:
    virtual ~FileWatcher() = default;
    
    // 监控文件
    virtual void watchFile(const std::string& file_path, 
                          std::function<void(const std::string&)> callback) = 0;
    
    // 停止监控文件
    virtual void unwatchFile(const std::string& file_path) = 0;
    
    // 启动/停止监控
    virtual void start() = 0;
    virtual void stop() = 0;
};
```

### PollingFileWatcher

基于轮询的文件监控器实现：

```cpp
class PollingFileWatcher : public FileWatcher {
public:
    // 构造函数，指定轮询间隔（毫秒）
    explicit PollingFileWatcher(int poll_interval_ms = 500);
    
    ~PollingFileWatcher() override;
    
    // FileWatcher接口实现
    void watchFile(const std::string& file_path, 
                   std::function<void(const std::string&)> callback) override;
    void unwatchFile(const std::string& file_path) override;
    void start() override;
    void stop() override;
    
    // 扩展功能
    void setPollInterval(int interval_ms);
    size_t getWatchedFileCount() const;
    void checkAllFiles();  // 手动检查所有文件
};
```

### FileWatcherFactory

工厂类，用于创建不同类型的文件监控器：

```cpp
class FileWatcherFactory {
public:
    // 创建轮询监控器
    static std::unique_ptr<FileWatcher> createPollingWatcher(int poll_interval_ms = 500);
    
    // 创建默认监控器
    static std::unique_ptr<FileWatcher> createDefault();
    
    // 未来扩展：
    // static std::unique_ptr<FileWatcher> createINotifyWatcher();  // Linux
    // static std::unique_ptr<FileWatcher> createKQueueWatcher();   // macOS
    // static std::unique_ptr<FileWatcher> createWin32Watcher();    // Windows
};
```

### 使用示例

```cpp
// 创建文件监控器
auto watcher = FileWatcherFactory::createDefault();

// 监控脚本文件
watcher->watchFile("game_logic.lua", [&](const std::string& file_path) {
    std::cout << "文件变化: " << file_path << std::endl;
    runtime.reloadScript("game_logic");
});

watcher->watchFile("config.lua", [&](const std::string& file_path) {
    std::cout << "配置文件变化: " << file_path << std::endl;
    runtime.reloadScript("config");
});

// 启动监控
watcher->start();

// 在程序退出前停止监控
watcher->stop();
```

## 线程安全说明

### 线程安全的组件

- **Logger**: 完全线程安全，可以从多个线程同时调用
- **MemoryAllocator**: 接口要求实现线程安全
- **PollingFileWatcher**: 内部使用互斥锁保护

### 非线程安全的组件

- **LuaRuntimeManager**: 不是线程安全的，应该在单线程中使用
- **Sol2状态**: Lua状态本身不是线程安全的

### 多线程使用建议

```cpp
// 每个线程使用独立的运行时管理器
thread_local LuaRuntimeManager runtime;

// 或者使用共享分配器但独立的运行时
auto shared_allocator = std::make_shared<MyAllocator>();

std::thread t1([=]() {
    LuaRuntimeManager runtime1(shared_allocator);
    // 线程1的逻辑...
});

std::thread t2([=]() {
    LuaRuntimeManager runtime2(shared_allocator);
    // 线程2的逻辑...
});
```

## 性能注意事项

### 内存分配

- 默认分配器使用标准库分配，适合大多数场景
- 对于高频分配场景，考虑使用内存池分配器
- 自定义分配器应该优化对齐和缓存局部性

### 热加载性能

- 文件检查频率影响CPU使用，根据需要调整轮询间隔
- 保护表机制会在重载时进行深拷贝，避免保护过大的表
- 考虑使用文件监控缓存来减少磁盘I/O

### 日志性能

- 编译时禁用日志可获得零运行时开销
- 运行时日志级别过滤比编译时过滤开销更大
- 避免在性能关键路径使用TRACE级别日志

### 示例性能优化

```cpp
// 高性能配置示例
class HighPerfAllocator : public MemoryAllocator {
    // 内存池 + 无锁分配器实现
};

// 禁用详细日志
Logger::setRuntimeLevel(LogLevel::WARN);

// 降低热加载检查频率
auto watcher = FileWatcherFactory::createPollingWatcher(2000);  // 2秒检查一次

// 最小化保护表
runtime.addProtectedTable("CriticalGameData");  // 只保护必要的数据
```