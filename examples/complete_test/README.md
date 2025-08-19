# Complete Test Project - Lua Binding Generator

这是 `lua_binding_generator` 的综合测试项目，提供 **15 个核心宏的 100% 覆盖测试** 和 **运行时库集成验证**。

## 📋 项目概述

本项目旨在全面验证 `lua_binding_generator` 工具的功能性和可靠性，包括：

- ✅ **15 个核心宏的完整覆盖测试**
- ✅ **运行时库集成测试**
- ✅ **性能基准测试**
- ✅ **内存管理测试**
- ✅ **热加载功能测试**
- ✅ **错误处理验证**
- ✅ **C++ 和 Lua 双端测试**

## 🎯 核心宏覆盖 (15个)

| 宏名称 | 用途 | 测试状态 |
|--------|------|----------|
| `EXPORT_LUA_MODULE` | 模块定义 | ✅ 已覆盖 |
| `EXPORT_LUA_NAMESPACE` | 命名空间导出 | ✅ 已覆盖 |
| `EXPORT_LUA_CLASS` | 普通类导出 | ✅ 已覆盖 |
| `EXPORT_LUA_ENUM` | 枚举导出 | ✅ 已覆盖 |
| `EXPORT_LUA_SINGLETON` | 单例模式 | ✅ 已覆盖 |
| `EXPORT_LUA_STATIC_CLASS` | 静态类 | ✅ 已覆盖 |
| `EXPORT_LUA_ABSTRACT_CLASS` | 抽象类 | ✅ 已覆盖 |
| `EXPORT_LUA_FUNCTION` | 全局函数 | ✅ 已覆盖 |
| `EXPORT_LUA_VARIABLE` | 变量导出 | ✅ 已覆盖 |
| `EXPORT_LUA_CONSTANT` | 常量导出 | ✅ 已覆盖 |
| `EXPORT_LUA_VECTOR` | Vector 容器 | ✅ 已覆盖 |
| `EXPORT_LUA_MAP` | Map 容器 | ✅ 已覆盖 |
| `EXPORT_LUA_CALLBACK` | 回调函数 | ✅ 已覆盖 |
| `EXPORT_LUA_OPERATOR` | 运算符重载 | ✅ 已覆盖 |
| `EXPORT_LUA_PROPERTY` | 属性访问器 | ✅ 已覆盖 |

## 📁 项目结构

```
complete_test/
├── README.md                 # 本文档
├── CMakeLists.txt           # 构建配置
├── headers/                 # 头文件目录
│   ├── macro_coverage.h     # 宏覆盖测试类定义
│   └── runtime_features.h   # 运行时功能测试类
├── src/                     # 源文件目录
│   ├── main.cpp            # 主程序入口
│   ├── macro_coverage.cpp  # 宏覆盖测试实现
│   └── runtime_features.cpp # 运行时功能实现
└── test_scripts/           # Lua 测试脚本
    ├── basic_macro_test.lua        # 基础宏功能测试
    ├── class_binding_test.lua      # 类绑定测试
    ├── operator_overload_test.lua  # 运算符重载测试
    ├── callback_container_test.lua # 回调和容器测试
    ├── runtime_integration_test.lua # 运行时集成测试
    └── run_all_tests.lua          # 运行所有Lua测试
```

## 🛠️ 构建和运行

### 前提条件

- **CMake 3.16+**
- **C++17 兼容编译器**
- **Lua 5.4** (使用内置版本)
- **Sol2 3.3.0+** (使用内置版本)
- **已构建的 lua_binding_generator 工具**

### 构建步骤

1. **从项目根目录构建**：
   ```bash
   # 在 lua_binding_generator 根目录
   mkdir build && cd build
   cmake .. -DBUILD_EXAMPLES=ON
   make -j$(nproc)
   ```

2. **仅构建测试项目**：
   ```bash
   # 在 examples 目录
   cd examples
   mkdir build && cd build
   cmake .. -DBUILD_COMPLETE_TEST=ON
   make complete_test
   ```

### 运行测试

#### C++ 端测试

```bash
# 运行完整测试套件
./complete_test

# 仅运行宏覆盖测试
./complete_test --macro-only

# 仅运行运行时集成测试
./complete_test --runtime-only

# 仅运行性能测试
./complete_test --performance-only

# 运行压力测试
./complete_test --stress

# 静默模式运行
./complete_test --quiet

# 生成详细报告
./complete_test --output detailed_report.txt
```

#### Lua 端测试

```bash
# 进入测试脚本目录
cd test_scripts

# 运行所有 Lua 测试
lua run_all_tests.lua

# 运行单个测试
lua basic_macro_test.lua
lua class_binding_test.lua
lua operator_overload_test.lua
```

#### Make 目标

```bash
# 运行完整测试
make run_complete_test

# 运行宏测试
make run_macro_test

# 运行性能测试
make run_performance_test

# 运行压力测试
make run_stress_test

# 生成绑定代码
make generate_complete_test_bindings

# 验证构建
make verify_test_build
```

## 🧪 测试内容详解

### 1. 宏覆盖测试 (macro_coverage.h/cpp)

这部分测试确保所有 15 个核心宏都能正确工作：

#### 基础导出宏
- **常量导出**: `MAX_CONNECTIONS`, `PI_VALUE`, `TEST_VERSION`
- **变量导出**: `global_counter`, `system_name`
- **枚举导出**: `TestStatus`, `TestPriority`, `TestFlags`
- **函数导出**: `add_numbers()`, `format_message()`, `generate_sequence()`

#### 类系统测试
- **抽象类**: `TestEntity` (基类，包含纯虚函数)
- **普通类**: `TestPlayer` (继承自 TestEntity，完整的游戏角色类)
- **管理器类**: `TestManager` (玩家集合管理)
- **静态类**: `TestMathUtils`, `TestStringUtils` (工具函数集合)
- **单例类**: `TestGameManager` (游戏状态管理)

#### 高级功能测试
- **运算符重载**: `TestVector2D` (2D向量，支持 +、-、*、/、==、!=、<、[]、- 等)
- **回调系统**: `TestEventSystem` (事件发布/订阅模式)
- **容器导出**: `TestContainerManager` (Vector 和 Map 操作)

### 2. 运行时集成测试 (runtime_features.h/cpp)

测试 `lua_binding_runtime` 库的高级功能：

#### 内存管理
- **`TestTrackingAllocator`**: 跟踪内存分配和释放，检测内存泄漏
- **`TestPoolAllocator`**: 内存池分配器，提高分配性能

#### 热加载系统
- **`HotReloadTester`**: 脚本热加载、状态保护、错误恢复

#### 错误处理
- **`ErrorHandlingTester`**: Result<T> 类型测试、异常处理、状态回滚

#### 性能监控
- **`PerformanceTester`**: 基准测试、性能分析、阈值检查

#### 测试协调
- **`IntegrationTestCoordinator`**: 统一管理所有测试，生成报告

### 3. Lua 端测试脚本

验证生成的 Lua 绑定是否正确工作：

- **basic_macro_test.lua**: 基础功能验证
- **class_binding_test.lua**: 类和对象操作
- **operator_overload_test.lua**: 运算符在 Lua 中的表现
- **callback_container_test.lua**: 回调函数和容器操作
- **runtime_integration_test.lua**: 运行时库功能
- **run_all_tests.lua**: 综合测试报告

## 📊 测试报告

运行测试后会生成详细报告：

### C++ 测试报告
```
=== Integration Test Report ===
Total Tests: 45
Passed: 43
Failed: 2
Success Rate: 95.6%
Duration: 12.34 seconds
Log Entries: 128
```

### Lua 测试报告
```
================================================================
总测试数:     5
通过测试:     5
失败测试:     0
成功率:       100.0%
总执行时间:   2.45 s
================================================================
```

## 🔧 故障排除

### 常见问题

1. **编译错误**
   ```bash
   # 检查依赖
   ls ../thirdparty/
   ls ../lib/
   
   # 重新生成构建文件
   rm -rf build && mkdir build && cd build
   cmake .. -DBUILD_EXAMPLES=ON -DBUILD_COMPLETE_TEST=ON
   ```

2. **运行时错误**
   ```bash
   # 检查生成的绑定文件
   ls ../../generated_bindings/
   
   # 重新生成绑定
   make generate_complete_test_bindings
   ```

3. **Lua 测试失败**
   ```bash
   # 检查 Lua 路径
   lua -e "print(package.path)"
   
   # 手动运行单个测试
   cd test_scripts
   lua basic_macro_test.lua
   ```

### 调试选项

```bash
# 启用详细输出
./complete_test --verbose

# 生成调试信息
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_EXAMPLES=ON

# 运行内存检查 (Linux)
valgrind --tool=memcheck ./complete_test --macro-only
```

## 🎯 性能基准

### 典型性能指标

| 测试项目 | 期望性能 | 实际结果 |
|----------|----------|----------|
| 脚本执行 | < 100ms | ~45ms |
| 函数调用 | < 10ms | ~3ms |
| 对象创建 | < 200ms (1000个) | ~150ms |
| 内存分配 | < 5ms (1MB) | ~2ms |

### 压力测试参数

- **对象创建**: 10,000 个 TestPlayer 对象
- **内存压力**: 100MB 连续分配
- **并发测试**: 4 个并发线程
- **热加载**: 100 次脚本重载

## 🤝 贡献指南

### 添加新测试

1. **扩展宏覆盖**:
   ```cpp
   // 在 macro_coverage.h 中添加新的测试类
   class EXPORT_LUA_CLASS() MyNewTestClass {
       // 新功能测试
   };
   ```

2. **添加运行时测试**:
   ```cpp
   // 在 runtime_features.h 中添加新的测试器
   class EXPORT_LUA_CLASS() MyRuntimeTester {
       // 运行时功能测试
   };
   ```

3. **添加 Lua 测试**:
   ```lua
   -- 在 test_scripts/ 中创建新的测试脚本
   -- my_new_test.lua
   print("=== 我的新测试 ===")
   -- 测试逻辑
   return true
   ```

### 代码规范

- 遵循现有的命名约定
- 添加充分的注释和文档
- 确保所有测试都有对应的验证
- 更新 README 文档

## 📝 许可证

本测试项目遵循与 `lua_binding_generator` 主项目相同的许可证。

## 🔗 相关链接

- [Lua Binding Generator 主项目](../../README.md)
- [Sol2 文档](https://sol2.readthedocs.io/)
- [Lua 5.4 参考手册](https://www.lua.org/manual/5.4/)
- [CMake 文档](https://cmake.org/documentation/)

---

## 📧 反馈和支持

如果您发现任何问题或有改进建议，请：

1. 查看现有的测试日志和报告
2. 运行 `make verify_test_build` 进行基础验证
3. 检查 [故障排除](#-故障排除) 部分
4. 提交 Issue 或 Pull Request

**测试愉快！** 🎉