# Lua Binding Generator

[English](./README_EN.md) | [中文](./README.md)

## 项目简介

Lua Binding Generator 是一个现代化的 C++ 到 Lua 绑定生成工具，专为简化开发流程而设计。通过智能 AST 分析和零配置理念，让 C++ 代码的 Lua 绑定生成变得前所未有的简单和高效。

### 为什么选择 Lua Binding Generator？

🚀 **零配置使用** - 大部分场景只需要无参数宏，告别繁琐配置  
🧠 **智能推导** - 自动从 AST 推导所需信息，大幅减少手动输入  
⚡ **极致性能** - 硬编码生成器，高效快速  
🔄 **增量编译** - 智能缓存机制，显著减少重新生成时间  
🎯 **全面支持** - 涵盖所有现代 C++ 特性，包括模板、STL、回调等  
🛠️ **完全自包含** - 内置所有必要的第三方库，无需额外安装

### 快速开始

```cpp
// 1. 包含头文件
#include "export_macros.h"

// 2. 定义模块
EXPORT_LUA_MODULE(GameCore)

// 3. 导出类（自动推导所有公共成员）
class EXPORT_LUA_CLASS() Player {
public:
    Player(const std::string& name, int level);
    
    // 这些方法会自动导出，无需额外配置
    std::string getName() const;        // 自动推导为只读属性 "name"
    void setName(const std::string& name);
    int getLevel() const;               // 自动推导为只读属性 "level"
    void levelUp();
};

// 4. 导出函数
EXPORT_LUA_FUNCTION()
double calculateDistance(double x1, double y1, double x2, double y2);

// 5. 生成绑定
// ./lua_binding_generator examples/your_code.h
```

### 核心特性

🔍 **自动推导**：
- 类名、方法名、函数名自动从代码中提取
- C++ 命名空间自动映射到 Lua 命名空间  
- get/set 方法自动配对为 Lua 属性
- STL 容器和模板参数自动识别

⚡ **增量编译**：
- 基于文件内容哈希的智能缓存
- 只重新生成变更的文件
- 大项目中显著减少重新生成时间

🎯 **并行处理**：
- 多线程并行分析和生成
- 智能任务分配和负载均衡

## 支持的宏列表

### 核心宏（15个）

| 宏名                           | 用途         | 示例                                              |
| ------------------------------ | ------------ | ------------------------------------------------- |
| `EXPORT_LUA_MODULE`            | 模块定义     | `EXPORT_LUA_MODULE(GameCore)`                     |
| `EXPORT_LUA_CLASS`             | 普通类导出   | `class EXPORT_LUA_CLASS() Player {}`              |
| `EXPORT_LUA_SINGLETON`         | 单例类导出   | `class EXPORT_LUA_SINGLETON() GameManager {}`     |
| `EXPORT_LUA_STATIC_CLASS`      | 静态类导出   | `class EXPORT_LUA_STATIC_CLASS() MathUtils {}`    |
| `EXPORT_LUA_ABSTRACT_CLASS`    | 抽象类导出   | `class EXPORT_LUA_ABSTRACT_CLASS() Component {}`  |
| `EXPORT_LUA_ENUM`              | 枚举导出     | `enum class EXPORT_LUA_ENUM() Status {}`          |
| `EXPORT_LUA_FUNCTION`          | 函数导出     | `EXPORT_LUA_FUNCTION() int calc();`               |
| `EXPORT_LUA_VARIABLE`          | 变量导出     | `EXPORT_LUA_VARIABLE() static int level;`         |
| `EXPORT_LUA_CONSTANT`          | 常量导出     | `EXPORT_LUA_CONSTANT() const int MAX = 100;`      |
| `EXPORT_LUA_VECTOR`            | Vector容器   | `EXPORT_LUA_VECTOR(int)`                          |
| `EXPORT_LUA_MAP`               | Map容器      | `EXPORT_LUA_MAP(std::string, int)`                |
| `EXPORT_LUA_CALLBACK`          | 回调函数导出 | `EXPORT_LUA_CALLBACK() std::function<void()> cb;` |
| `EXPORT_LUA_OPERATOR`          | 运算符导出   | `EXPORT_LUA_OPERATOR(+) Vector operator+();`      |
| `EXPORT_LUA_TEMPLATE`          | 模板类导出   | `class EXPORT_LUA_TEMPLATE(T) Container {}`       |
| `EXPORT_LUA_IGNORE`            | 忽略导出     | `EXPORT_LUA_IGNORE() void internal();`            |

### 便利宏

| 宏名                            | 用途     | 等价于                                  |
| ------------------------------- | -------- | --------------------------------------- |
| `EXPORT_LUA_READONLY_PROPERTY`  | 只读属性 | `EXPORT_LUA_PROPERTY(access=readonly)`  |
| `EXPORT_LUA_READWRITE_PROPERTY` | 读写属性 | `EXPORT_LUA_PROPERTY(access=readwrite)` |

## 详细使用示例

### 零配置使用示例

以下示例展示了工具的使用方式：

```cpp
#include "export_macros.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

// 1. 模块定义 - 文件级别设置
EXPORT_LUA_MODULE(GameCore)

namespace game {

// 2. 枚举 - 自动导出所有值
enum class EXPORT_LUA_ENUM() PlayerStatus {
    ALIVE,      // 自动导出为 PlayerStatus.ALIVE
    DEAD,       // 自动导出为 PlayerStatus.DEAD
    RESPAWNING  // 自动导出为 PlayerStatus.RESPAWNING
};

// 3. 基础类 - 自动导出所有公共成员
class EXPORT_LUA_CLASS() Player {
public:
    // 构造函数 - 自动导出
    Player();
    Player(const std::string& name, int level);
    
    // 属性方法 - 自动识别为 Lua 属性
    std::string getName() const;           // 自动推导为只读属性 "name"
    void setName(const std::string& name);
    
    int getLevel() const;                  // 自动推导为只读属性 "level"
    
    int getHealth() const;                 // 自动推导为读写属性 "health"
    void setHealth(int health);
    
    // 普通方法 - 自动导出
    void attack(const std::string& target);
    bool isAlive() const;
    void levelUp();
    
    // 静态方法 - 自动导出
    static int getMaxLevel();
    static std::shared_ptr<Player> createDefault();
    
    // 不想导出的方法 - 明确标记忽略
    EXPORT_LUA_IGNORE()
    void debugInternalState();

private:
    std::string name_;    // 私有成员自动忽略
    int level_;
    int health_;
};

// 4. 全局函数 - 自动推导函数名
EXPORT_LUA_FUNCTION()
double calculateDistance(double x1, double y1, double x2, double y2);

EXPORT_LUA_FUNCTION()
int calculateDamage(int attack, int defense);

// 5. 常量 - 自动推导为只读
EXPORT_LUA_CONSTANT()
static const int MAX_PLAYERS = 100;

EXPORT_LUA_CONSTANT()
static const double PI = 3.14159;

} // namespace game
```

### 高级特性使用示例

```cpp
#include "export_macros.h"

EXPORT_LUA_MODULE(AdvancedFeatures)

namespace advanced {

// 1. 单例类 - 自动识别单例模式
class EXPORT_LUA_SINGLETON() GameManager {
public:
    static GameManager& getInstance();     // 自动识别为单例访问方法
    
    void startGame();
    void stopGame();
    bool isGameRunning() const;
    
    void addPlayer(std::shared_ptr<game::Player> player);
    std::vector<std::shared_ptr<game::Player>> getAllPlayers() const;

private:
    GameManager() = default;               // 私有构造函数自动忽略
    bool game_running_ = false;
    std::vector<std::shared_ptr<game::Player>> players_;
};

// 2. 静态工具类 - 只导出静态成员
class EXPORT_LUA_STATIC_CLASS() MathUtils {
public:
    static double clamp(double value, double min, double max);
    static double lerp(double a, double b, double t);
    static int random(int min, int max);
    static double randomFloat(double min, double max);
    
    // 静态常量
    static const double PI;
    static const double E;
};

// 3. 抽象基类 - 支持多态
class EXPORT_LUA_ABSTRACT_CLASS() Component {
public:
    Component() = default;
    virtual ~Component() = default;
    
    // 纯虚函数 - 不直接绑定到 Lua
    virtual void initialize() = 0;
    virtual void update(double deltaTime) = 0;
    virtual void destroy() = 0;
    
    // 普通虚函数 - 正常导出
    virtual bool isActive() const;
    virtual void setActive(bool active);

protected:
    bool active_ = true;
};

// 4. 运算符重载 - 自动映射到 Lua 元方法
class EXPORT_LUA_CLASS() Vector2D {
public:
    Vector2D();
    Vector2D(double x, double y);
    
    // 属性访问
    double getX() const;
    void setX(double x);
    double getY() const;
    void setY(double y);
    
    // 算术运算符 - 自动映射到 Lua 元方法
    EXPORT_LUA_OPERATOR(+)
    Vector2D operator+(const Vector2D& other) const;    // 映射到 __add
    
    EXPORT_LUA_OPERATOR(-)
    Vector2D operator-(const Vector2D& other) const;    // 映射到 __sub
    
    EXPORT_LUA_OPERATOR(*)
    Vector2D operator*(double scalar) const;            // 映射到 __mul
    
    // 比较运算符
    EXPORT_LUA_OPERATOR(==)
    bool operator==(const Vector2D& other) const;       // 映射到 __eq
    
    // 下标运算符
    EXPORT_LUA_OPERATOR([])
    double operator[](int index) const;                 // 映射到 __index

private:
    double x_, y_;
};

} // namespace advanced

// 5. STL 容器导出 - 自动生成完整绑定
EXPORT_LUA_VECTOR(int)
EXPORT_LUA_VECTOR(std::string)
EXPORT_LUA_VECTOR(std::shared_ptr<game::Player>)
EXPORT_LUA_MAP(std::string, int)
EXPORT_LUA_MAP(int, std::shared_ptr<game::Player>)

namespace events {

// 6. 事件系统 - 回调函数自动推导
class EXPORT_LUA_CLASS() EventSystem {
public:
    EventSystem();
    
    // 回调函数 - 自动推导参数类型和数量
    EXPORT_LUA_CALLBACK()
    std::function<void()> OnGameStart;                   // 无参数回调
    
    EXPORT_LUA_CALLBACK()
    std::function<void(std::shared_ptr<game::Player>)> OnPlayerJoin;  // 单参数回调
    
    EXPORT_LUA_CALLBACK()
    std::function<bool(const std::string&, double)> OnValidateAction;  // 有返回值回调
    
    // 事件触发方法
    void triggerGameStart();
    void triggerPlayerJoin(std::shared_ptr<game::Player> player);
    bool validateAction(const std::string& action, double value);

private:
    bool initialized_ = false;
};

} // namespace events
```

## 编译和使用

### 1. 系统要求

- **C++17** 兼容编译器（GCC 7+, Clang 7+, MSVC 2019+）
- **CMake 3.16+**
- **操作系统**: Linux, macOS, Windows

项目已自包含所有必要的第三方库源码，无需额外安装系统依赖。

### 2. 编译工具

```bash
# 从项目根目录构建
mkdir build && cd build
cmake ..
make

# 或者使用自动化脚本
./scripts/build_and_test_all.sh
```

### 3. 使用自动化脚本

#### Unix/Linux/macOS

```bash
# 完整的构建和测试流程
./scripts/build_and_test_all.sh

# 带清理选项
./scripts/build_and_test_all.sh --clean

# 清理第三方库构建产物（释放磁盘空间）
./scripts/build_and_test_all.sh --clean-thirdparty

# 完全清理第三方库（包括可执行文件）
./scripts/build_and_test_all.sh --clean-thirdparty-full

# 详细输出
./scripts/build_and_test_all.sh --verbose

# 查看帮助
./scripts/build_and_test_all.sh --help
```

#### Windows

```cmd
# 完整的构建和测试流程
scripts\build_and_test_all.bat

# 带清理选项
scripts\build_and_test_all.bat /clean

# 详细输出
scripts\build_and_test_all.bat /verbose

# 查看帮助
scripts\build_and_test_all.bat /help
```

### 4. 第三方库清理工具

项目提供了专门的脚本来清理第三方库的构建产物，帮助节省磁盘空间：

```bash
# 轻量级清理（CMake缓存、临时文件）
./scripts/clean_thirdparty.sh

# 完全清理（所有构建产物）
./scripts/clean_thirdparty.sh --level=full

# 预览清理内容（不实际删除）
./scripts/clean_thirdparty.sh --dry-run

# 清理特定库
./scripts/clean_thirdparty.sh --library=llvm

# 创建备份
./scripts/clean_thirdparty.sh --level=full --backup
```

### 5. 生成 Lua 绑定

```bash
# 最简形式
./lua_binding_generator examples/simple_example.h

# 处理多个文件
./lua_binding_generator examples/*.h

# 指定输出目录和模块名
./lua_binding_generator --output-dir=bindings --module-name=GameCore examples/*.h

# 启用详细输出
./lua_binding_generator --verbose examples/*.h

# 强制重新生成所有文件
./lua_binding_generator --force-rebuild examples/*.h
```

### 6. 命令行选项

| 选项              | 默认值               | 说明                 |
| ----------------- | -------------------- | -------------------- |
| `--output-dir`    | `generated_bindings` | 输出目录             |
| `--module-name`   | 自动推导             | Lua 模块名           |
| `--incremental`   | `true`               | 启用增量编译         |
| `--force-rebuild` | `false`              | 强制重新生成所有文件 |
| `--parallel`      | `true`               | 启用并行处理         |
| `--max-threads`   | `0`（自动）          | 最大线程数           |
| `--verbose`       | `false`              | 详细输出             |
| `--show-stats`    | `false`              | 显示统计信息         |

## 自动推导功能

### 命名空间推导

优先级：明确指定 > C++ namespace > EXPORT_LUA_MODULE > 全局

```cpp
EXPORT_LUA_MODULE(GameCore)

namespace game {
    class EXPORT_LUA_CLASS() Player {};  // 推导为 game.Player
    
    class EXPORT_LUA_CLASS(namespace=combat) Weapon {};  // 明确指定为 combat.Weapon
}
```

### 属性推导

自动识别 get/set 方法配对：

```cpp
class EXPORT_LUA_CLASS() Player {
public:
    int getHealth() const;      // 推导为只读属性 "health"
    void setHealth(int health);
    
    int getLevel() const;       // 推导为只读属性 "level"
    
    double getMana() const;     // 推导为读写属性 "mana"
    void setMana(double mana);
};
```

### 枚举值推导

```cpp
enum class EXPORT_LUA_ENUM() Status {
    ACTIVE = 1,     // 自动导出为 Status.ACTIVE = 1
    INACTIVE = 2,   // 自动导出为 Status.INACTIVE = 2
    PENDING         // 自动推导值为 3
};
```

### 单例模式检测

自动识别常见的单例访问方法：

```cpp
class EXPORT_LUA_SINGLETON() GameManager {
public:
    static GameManager& getInstance();  // 自动识别
    static GameManager& instance();     // 或者这个
    static GameManager* get();          // 或者这个
};
```

## 生成的绑定代码示例

对于上面的 `Player` 类，工具会生成类似这样的 Sol2 绑定代码：

```cpp
#include <sol/sol.hpp>
#include "game_player.h"

void register_GameCore_bindings(sol::state& lua) {
    // 创建命名空间
    auto game_ns = lua["game"].get_or_create<sol::table>();
    
    // 注册 PlayerStatus 枚举
    game_ns.new_enum<game::PlayerStatus>("PlayerStatus",
        "ALIVE", game::PlayerStatus::ALIVE,
        "DEAD", game::PlayerStatus::DEAD,
        "RESPAWNING", game::PlayerStatus::RESPAWNING
    );
    
    // 注册 Player 类
    auto player_type = game_ns.new_usertype<game::Player>("Player",
        // 构造函数
        sol::constructors<game::Player(), game::Player(const std::string&, int)>(),
        
        // 属性（自动识别的 get/set 配对）
        "name", sol::property(&game::Player::getName, &game::Player::setName),
        "level", sol::readonly_property(&game::Player::getLevel),
        "health", sol::property(&game::Player::getHealth, &game::Player::setHealth),
        
        // 方法
        "attack", &game::Player::attack,
        "isAlive", &game::Player::isAlive,
        "levelUp", &game::Player::levelUp,
        
        // 静态方法
        "getMaxLevel", &game::Player::getMaxLevel,
        "createDefault", &game::Player::createDefault
    );
    
    // 注册全局函数到 game 命名空间
    game_ns.set_function("calculateDistance", &game::calculateDistance);
    game_ns.set_function("calculateDamage", &game::calculateDamage);
    
    // 注册常量
    game_ns["MAX_PLAYERS"] = game::MAX_PLAYERS;
    game_ns["PI"] = game::PI;
}
```

## 项目结构

```
lua_binding_generator/
├── CMakeLists.txt                      # 主构建配置
├── main.cpp                            # 工具主程序入口
├── README.md                           # 项目文档（中文）
├── README_EN.md                        # 项目文档（英文）
├── include/                            # 头文件目录
│   ├── export_macros.h                 # 智能推导宏定义
│   ├── ast_visitor.h                   # AST 访问器
│   ├── direct_binding_generator.h      # 硬编码绑定生成器
│   ├── smart_inference_engine.h        # 智能推导引擎
│   ├── incremental_generator.h         # 增量编译系统
│   ├── compiler_detector.h             # 编译器检测
│   ├── dynamic_compilation_database.h  # 动态编译数据库
│   └── logger.h                        # 日志系统
├── src/                                # 源文件目录
│   ├── ast_visitor.cpp
│   ├── direct_binding_generator.cpp
│   ├── smart_inference_engine.cpp
│   ├── incremental_generator.cpp
│   ├── compiler_detector.cpp
│   ├── dynamic_compilation_database.cpp
│   └── logger.cpp
├── examples/                           # 示例代码
│   ├── simple_example.h                # 简单使用示例
│   ├── simple_example.cpp
│   ├── simple_main.cpp
│   ├── game_engine.h                   # 游戏引擎示例
│   ├── game_engine.cpp
│   ├── game_engine_main.cpp
│   ├── comprehensive_test.h            # 完整特性测试
│   ├── comprehensive_test.cpp
│   ├── comprehensive_main.cpp
│   └── scripts/                        # Lua 测试脚本
│       ├── test_simple.lua
│       ├── test_game_engine.lua
│       ├── test_comprehensive.lua
│       ├── test_bindings_integration.lua
│       └── README.md
├── scripts/                            # 自动化脚本
│   ├── build_and_test_all.sh           # Unix/Linux/macOS 构建脚本
│   ├── build_and_test_all.bat          # Windows 构建脚本
│   ├── clean_thirdparty.sh             # Unix 第三方清理脚本
│   ├── clean_thirdparty.bat            # Windows 第三方清理脚本
│   └── README.md                       # 脚本使用说明
├── generated_bindings/                 # 生成的绑定文件（运行时创建）
│   └── generated_module_bindings.cpp
├── build/                              # 构建目录（运行时创建）
└── thirdparty/                         # 第三方库（自包含）
    ├── llvm-20.1.8/                    # LLVM 编译器基础设施
    ├── clang-tools-extra-20.1.8.src/   # Clang 工具扩展
    ├── lua-5.4.8/                      # Lua 解释器
    ├── sol2-3.3.0/                     # Sol2 C++ Lua 绑定库
    ├── spdlog-1.15.3/                  # 高性能日志库
    └── zstd-1.5.7/                     # 压缩库
```


## 性能优化

### 增量编译

- 基于文件内容哈希的智能缓存
- 依赖关系跟踪和传播
- 大项目中可显著减少重新生成时间

### 并行处理

- 多线程并行分析和生成
- 智能任务分配
- 支持大型项目的快速处理

### 内存优化

- 移除模板解析开销
- 直接字符串操作
- 优化的 AST 遍历

## 故障排除

### 常见问题

1. **编译器不支持 `__attribute__((annotate))`**
   - 使用 Clang 编译器
   - 或者升级到支持该特性的 GCC 版本

2. **未找到导出项**
   - 确保使用了 `EXPORT_LUA_*` 宏
   - 检查宏是否在正确的位置

3. **增量编译不工作**
   - 删除缓存文件重试：`rm .lua_binding_cache`
   - 使用 `--force-rebuild` 强制重新生成

4. **生成的绑定编译失败**
   - 检查 Sol2 版本兼容性
   - 查看生成的代码中的错误信息

5. **第三方库占用过多磁盘空间**
   - 使用清理脚本：`./scripts/clean_thirdparty.sh --level=full`
   - 定期清理构建产物

### 调试选项

```bash
# 启用详细输出
./lua_binding_generator --verbose --show-stats examples/*.h

# 强制重新生成（忽略缓存）
./lua_binding_generator --force-rebuild examples/*.h

# 禁用并行处理（便于调试）
./lua_binding_generator --parallel=false examples/*.h
```

## 总结

lua_binding_generator 实现了以下关键目标：

1. **零配置使用** - 大部分场景只需要无参数宏
2. **智能推导** - 自动从 AST 推导大部分配置信息  
3. **全面覆盖** - 支持所有现代 C++ 特性
4. **高性能** - 硬编码生成器，增量编译高效处理
5. **完全自包含** - 内置所有依赖，简化部署
6. **跨平台支持** - Linux、macOS、Windows 全平台支持

lua_binding_generator 真正实现了"让使用者的心理负担降到最低，并最大限度的提升工作效率"的设计目标。

## 贡献

欢迎提交 Issue 和 Pull Request！

## 许可证

本项目使用 MIT 许可证。

---

📖 **Documentation**: [English](./README_EN.md) | [中文](./README.md)