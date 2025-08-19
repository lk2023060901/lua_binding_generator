# 快速入门指南

本指南将帮助您在15分钟内开始使用Lua Binding Runtime库。

## 📋 前提条件

- C++17编译器（GCC 7+, Clang 6+, MSVC 2019+）
- CMake 3.16+
- Lua 5.1+（建议5.4）
- Sol2库

## 🚀 5分钟快速开始

### 第1步：构建库

```bash
cd lua_binding_generator/lib
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 第2步：创建您的第一个项目

创建`my_project.cpp`：

```cpp
#include "lua_binding_runtime.h"
#include <iostream>

// 简单的C++类
class Calculator {
public:
    int add(int a, int b) { return a + b; }
    int multiply(int a, int b) { return a * b; }
};

// 绑定函数
void register_calculator_bindings(sol::state& lua) {
    lua.new_usertype<Calculator>("Calculator",
        sol::constructors<Calculator()>(),
        "add", &Calculator::add,
        "multiply", &Calculator::multiply
    );
    
    std::cout << "Calculator绑定注册完成" << std::endl;
}

int main() {
    using namespace lua_runtime;
    
    // 1. 创建运行时管理器
    LuaRuntimeManager runtime;
    
    // 2. 注册绑定
    auto result = runtime.registerBindings(register_calculator_bindings);
    if (result.isError()) {
        std::cerr << "绑定失败: " << result.error().message << std::endl;
        return 1;
    }
    
    // 3. 执行Lua代码
    auto script_result = runtime.executeScript(R"(
        local calc = Calculator.new()
        local sum = calc:add(10, 20)
        local product = calc:multiply(5, 6)
        print("10 + 20 = " .. sum)
        print("5 * 6 = " .. product)
        return sum + product
    )");
    
    if (script_result.isSuccess()) {
        auto lua_object = script_result.value();
        if (lua_object.is<int>()) {
            std::cout << "Lua返回值: " << lua_object.as<int>() << std::endl;
        }
    } else {
        std::cerr << "脚本执行失败: " << script_result.error().message << std::endl;
    }
    
    return 0;
}
```

### 第3步：编译并运行

```bash
g++ -std=c++17 my_project.cpp -I../include -L./lib -llua_binding_runtime -llua -o my_project
./my_project
```

预期输出：
```
Calculator绑定注册完成
10 + 20 = 30
5 * 6 = 30
Lua返回值: 60
```

## 🔥 热加载示例（10分钟）

### 第1步：创建热加载项目

创建`hot_reload_example.cpp`：

```cpp
#include "lua_binding_runtime.h"
#include <iostream>
#include <thread>
#include <chrono>

void register_math_bindings(sol::state& lua) {
    lua["cpp_add"] = [](int a, int b) { return a + b; };
    lua["cpp_multiply"] = [](int a, int b) { return a * b; };
}

int main() {
    using namespace lua_runtime;
    
    LuaRuntimeManager runtime;
    runtime.registerBindings(register_math_bindings);
    
    // 创建初始脚本文件
    std::ofstream script_file("math_operations.lua");
    script_file << R"(
-- math_operations.lua v1.0
print("数学操作脚本 v1.0 已加载")

function calculate()
    local a, b = 10, 5
    local sum = cpp_add(a, b)
    local product = cpp_multiply(a, b)
    print("结果: " .. a .. " + " .. b .. " = " .. sum)
    print("结果: " .. a .. " * " .. b .. " = " .. product)
    return sum, product
end

function get_version()
    return "1.0"
end
)";
    script_file.close();
    
    // 注册热加载脚本
    runtime.registerHotReloadScript("math_ops", "math_operations.lua");
    
    // 设置回调
    runtime.setPostReloadCallback([](const HotReloadEvent& event) {
        if (event.result == HotReloadResult::SUCCESS) {
            std::cout << "✅ 脚本重载成功: " << event.script_name << std::endl;
        } else {
            std::cout << "❌ 脚本重载失败: " << event.error_message << std::endl;
        }
    });
    
    std::cout << "热加载示例启动！" << std::endl;
    std::cout << "请修改 math_operations.lua 文件来测试热加载" << std::endl;
    std::cout << "按 Ctrl+C 退出" << std::endl;
    
    // 主循环
    for (int i = 0; i < 30; ++i) {  // 运行30秒
        // 检查热加载
        runtime.checkAndReloadScripts();
        
        // 调用Lua函数
        auto version_result = runtime.callLuaFunction<std::string>("get_version");
        if (version_result.isSuccess()) {
            std::cout << "[" << i << "] 脚本版本: " << version_result.value() << std::endl;
        }
        
        auto calc_result = runtime.callLuaFunction<void>("calculate");
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
```

### 第2步：编译并运行

```bash
g++ -std=c++17 hot_reload_example.cpp -I../include -L./lib -llua_binding_runtime -llua -o hot_reload_example
./hot_reload_example
```

### 第3步：测试热加载

程序运行时，编辑`math_operations.lua`文件：

```lua
-- math_operations.lua v2.0 (修改版本)
print("数学操作脚本 v2.0 已加载 - 新功能!")

function calculate()
    local a, b = 15, 3  -- 修改数值
    local sum = cpp_add(a, b)
    local product = cpp_multiply(a, b)
    local difference = a - b  -- 新增计算
    
    print("更新的结果: " .. a .. " + " .. b .. " = " .. sum)
    print("更新的结果: " .. a .. " * " .. b .. " = " .. product)
    print("新增功能: " .. a .. " - " .. b .. " = " .. difference)
    return sum, product, difference
end

function get_version()
    return "2.0"  -- 更新版本号
end
```

保存文件后，您将看到：
```
✅ 脚本重载成功: math_ops
[5] 脚本版本: 2.0
更新的结果: 15 + 3 = 18
更新的结果: 15 * 3 = 45
新增功能: 15 - 3 = 12
```

## 🎯 实用场景示例

### 游戏配置热加载

```cpp
// game_config_example.cpp
#include "lua_binding_runtime.h"

int main() {
    using namespace lua_runtime;
    
    LuaRuntimeManager runtime;
    
    // 保护玩家数据不被重载覆盖
    runtime.addProtectedTable("PlayerData");
    runtime.addProtectedTable("GameState");
    
    // 注册配置脚本
    runtime.registerHotReloadScript("config", "game_config.lua");
    
    // 初始化玩家数据
    runtime.executeScript(R"(
        PlayerData = {
            level = 1,
            experience = 0,
            gold = 100
        }
        
        GameState = {
            current_map = "tutorial",
            enemies_defeated = 0
        }
    )");
    
    // 设置配置重载回调
    runtime.setPostReloadCallback([&](const HotReloadEvent& event) {
        if (event.script_name == "config" && event.result == HotReloadResult::SUCCESS) {
            std::cout << "游戏配置已更新！" << std::endl;
            
            // 应用新配置
            runtime.executeScript(R"(
                if apply_config then
                    apply_config()
                end
            )");
        }
    });
    
    // 游戏主循环模拟
    for (int frame = 0; frame < 100; ++frame) {
        runtime.checkAndReloadScripts();
        
        // 模拟游戏逻辑
        if (frame % 20 == 0) {
            runtime.executeScript(R"(
                print("=== 游戏状态 ===")
                print("玩家等级: " .. PlayerData.level)
                print("经验值: " .. PlayerData.experience)
                print("金币: " .. PlayerData.gold)
                print("当前地图: " .. GameState.current_map)
                
                if GameConfig then
                    print("难度: " .. GameConfig.difficulty)
                    print("怪物生成率: " .. GameConfig.monster_spawn_rate)
                end
            )");
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return 0;
}
```

配置文件`game_config.lua`：

```lua
-- game_config.lua
print("加载游戏配置...")

GameConfig = {
    difficulty = "normal",
    monster_spawn_rate = 1.0,
    experience_multiplier = 1.0,
    gold_drop_rate = 1.0,
    player_health = 100,
    player_mana = 50
}

function apply_config()
    print("应用游戏配置:")
    print("  难度: " .. GameConfig.difficulty)
    print("  经验倍数: " .. GameConfig.experience_multiplier)
    
    -- 根据难度调整玩家属性
    if GameConfig.difficulty == "easy" then
        PlayerData.gold = PlayerData.gold + 50
        print("  简单模式奖励: +50金币")
    elseif GameConfig.difficulty == "hard" then
        GameConfig.monster_spawn_rate = 2.0
        print("  困难模式: 怪物生成率翻倍")
    end
end

-- 自动应用配置
apply_config()
```

### 自定义内存分配器

```cpp
// memory_example.cpp
#include "lua_binding_runtime.h"

class LoggingAllocator : public lua_runtime::MemoryAllocator {
private:
    size_t total_allocated_ = 0;
    size_t allocation_count_ = 0;
    
public:
    void* allocate(size_t size, size_t alignment) override {
        void* ptr = std::aligned_alloc(alignment, size);
        if (ptr) {
            total_allocated_ += size;
            allocation_count_++;
            std::cout << "分配: " << size << " 字节, 总计: " << total_allocated_ << std::endl;
        }
        return ptr;
    }
    
    void deallocate(void* ptr, size_t size) override {
        if (ptr) {
            total_allocated_ -= size;
            std::cout << "释放: " << size << " 字节, 总计: " << total_allocated_ << std::endl;
            std::free(ptr);
        }
    }
    
    void* reallocate(void* ptr, size_t old_size, size_t new_size) override {
        void* new_ptr = std::realloc(ptr, new_size);
        if (new_ptr && old_size != new_size) {
            total_allocated_ = total_allocated_ - old_size + new_size;
            std::cout << "重分配: " << old_size << " -> " << new_size 
                     << " 字节, 总计: " << total_allocated_ << std::endl;
        }
        return new_ptr;
    }
    
    size_t getTotalAllocated() const { return total_allocated_; }
    size_t getAllocationCount() const { return allocation_count_; }
};

int main() {
    auto allocator = std::make_unique<LoggingAllocator>();
    auto* allocator_ptr = allocator.get();
    
    lua_runtime::LuaRuntimeManager runtime(std::move(allocator));
    
    // 执行一些内存分配操作
    runtime.executeScript(R"(
        local data = {}
        for i = 1, 100 do
            data[i] = "String " .. i
        end
        print("创建了100个字符串")
    )");
    
    std::cout << "最终内存统计:" << std::endl;
    std::cout << "  总分配: " << allocator_ptr->getTotalAllocated() << " 字节" << std::endl;
    std::cout << "  分配次数: " << allocator_ptr->getAllocationCount() << std::endl;
    
    return 0;
}
```

## 🔧 常见问题解决

### 编译错误

**问题**: 找不到Sol2头文件
```bash
fatal error: sol/sol.hpp: No such file or directory
```

**解决**: 确保Sol2在包含路径中
```bash
# 下载Sol2
wget https://github.com/ThePhD/sol2/releases/download/v3.3.0/sol.hpp
mkdir -p include/sol
mv sol.hpp include/sol/

# 或使用包管理器
apt-get install libsol2-dev  # Ubuntu
brew install sol2           # macOS
```

**问题**: 链接错误
```bash
undefined reference to lua_*
```

**解决**: 链接Lua库
```bash
# 安装Lua开发包
apt-get install liblua5.4-dev  # Ubuntu
brew install lua              # macOS

# 编译时链接
g++ ... -llua -ldl
```

### 运行时错误

**问题**: 脚本执行失败
```cpp
// 检查详细错误信息
auto result = runtime.executeScript("...");
if (result.isError()) {
    std::cerr << "错误类型: " << (int)result.error().type << std::endl;
    std::cerr << "错误信息: " << result.error().message << std::endl;
    std::cerr << "错误上下文: " << result.error().context << std::endl;
}
```

**问题**: 热加载不工作
```cpp
// 检查文件路径和权限
auto scripts = runtime.getRegisteredScripts();
for (const auto& script : scripts) {
    std::cout << "脚本: " << script.name << " -> " << script.file_path << std::endl;
    std::ifstream file(script.file_path);
    if (!file.good()) {
        std::cout << "文件不可访问: " << script.file_path << std::endl;
    }
}
```

## 📖 下一步

- 查看[完整API文档](API_REFERENCE.md)
- 运行[示例项目](../examples/)
- 了解[高级功能](ADVANCED_USAGE.md)
- 学习[性能优化](PERFORMANCE_GUIDE.md)

## 💡 提示

1. **日志调试**: 开启详细日志来调试问题
   ```cpp
   Logger::setRuntimeLevel(LogLevel::DEBUG);
   ```

2. **错误处理**: 总是检查Result返回值
   ```cpp
   auto result = runtime.executeScript("...");
   if (!result) {  // 简化的错误检查
       handle_error(result.error());
   }
   ```

3. **性能监控**: 使用自定义分配器监控内存使用
4. **热加载最佳实践**: 保护重要的游戏状态表，避免数据丢失

现在您已经掌握了基础用法，可以开始构建自己的Lua集成应用了！