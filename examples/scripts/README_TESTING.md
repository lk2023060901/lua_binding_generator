# Lua 测试脚本使用指南

## 🛠️ 修复内容

已修复所有 Lua 测试脚本中的问题：

### 1. 语法错误修复

**主要问题**: `run_all_tests.lua` 中使用了错误的字符串格式化语法
```lua
-- 错误写法：
"✅ PASSED: %s (%.3fs)" % duration

-- 正确写法：
string.format("✅ PASSED: %s (%.3fs)", description, duration)
```

### 2. 模块可用性检查

所有测试脚本现在都会在开始时检查必需的模块是否可用：

- `test_simple.lua` - 检查 `simple` 模块
- `test_game_engine.lua` - 检查 `engine` 模块  
- `test_comprehensive.lua` - 检查 `game` 模块
- `test_bindings_integration.lua` - 检查 `simple`, `engine`, `game` 模块

如果模块不可用，测试会优雅地退出并提供有用的错误信息。

### 3. 新增测试脚本

#### `test_framework_standalone.lua`
独立的测试框架验证脚本，不依赖任何绑定模块：
- 测试断言函数是否正常工作
- 验证错误检测机制
- 提供基准测试结果

#### `run_tests_safe.lua`
安全版本的测试运行器：
- 智能检查模块可用性
- 跳过不可用的测试
- 提供详细的状态报告
- 给出明确的下一步指导

## 🚀 使用方法

### 前提条件

1. **安装 Lua**:
   ```bash
   # macOS
   brew install lua
   
   # Ubuntu/Debian
   sudo apt-get install lua5.3
   
   # CentOS/RHEL
   sudo yum install lua
   ```

2. **验证安装**:
   ```bash
   lua -v
   ```

### 运行测试

#### 1. 验证测试框架本身
```bash
cd examples
lua scripts/test_framework_standalone.lua
```
预期输出：一些通过的测试和3个故意失败的测试来验证错误检测。

#### 2. 安全运行所有测试
```bash
cd examples  
lua scripts/run_tests_safe.lua
```
这会检查哪些模块可用并只运行相应的测试。

#### 3. 运行特定测试（需要绑定）
```bash
# 仅在生成并集成绑定后使用
lua scripts/test_simple.lua
lua scripts/test_game_engine.lua
lua scripts/test_comprehensive.lua
```

### 集成绑定后的完整流程

1. **生成绑定代码**:
   ```bash
   cd /path/to/lua_binding_generator
   ./build/lua_binding_generator --output-dir=generated_bindings examples/*.h
   ```

2. **集成到Sol2应用**:
   - 在C++应用中加载生成的绑定代码
   - 注册所有绑定函数到Lua状态
   - 确保模块正确映射（simple, engine, game等）

3. **运行完整测试套件**:
   ```bash
   # 在集成的应用中执行
   lua scripts/run_all_tests.lua
   ```

## 📋 测试覆盖

### Simple Example Tests (`test_simple.lua`)
- ✅ 枚举值验证
- ✅ 常量值检查  
- ✅ 全局函数调用
- ✅ Calculator 类的构造、方法、静态函数
- ✅ DataContainer 容器操作
- ✅ StringUtils 静态工具类
- ✅ 边界条件和错误处理

### Game Engine Tests (`test_game_engine.lua`)
- ✅ 复杂枚举类型
- ✅ Vector2 数学运算和运算符重载
- ✅ GameObject 层次结构和继承
- ✅ Time 管理系统
- ✅ 碰撞检测工具
- ✅ World 和 GameManager 单例模式
- ✅ 事件系统和回调函数

### Comprehensive Tests (`test_comprehensive.lua`)  
- ✅ 高级继承场景
- ✅ 复杂回调系统
- ✅ STL 容器绑定
- ✅ 智能指针集成
- ✅ 模板类实例
- ✅ 多参数事件
- ✅ 抽象基类

### Integration Tests (`test_bindings_integration.lua`)
- ✅ 内存管理验证
- ✅ 异常处理验证
- ✅ 类型安全检查
- ✅ 基础性能基准测试
- ✅ 线程安全模拟
- ✅ 跨模块兼容性

## 🔧 故障排除

### 常见错误

1. **"module not available"**
   - 确保已生成绑定代码
   - 检查Sol2集成是否正确
   - 验证模块注册函数被调用

2. **"attempt to call field 'function' (a nil value)"**
   - 检查函数是否有正确的EXPORT宏
   - 验证C++编译是否成功
   - 确保绑定注册完整

3. **类型不匹配错误**
   - 检查返回类型处理
   - 验证Sol2类型转换设置
   - 审查自定义类型注册

### 调试模式

在测试脚本中启用调试输出：
```lua
local DEBUG = true

function debug_print(msg)
    if DEBUG then
        print("[DEBUG] " .. msg)
    end
end
```

## 🎯 测试状态

- ✅ **测试框架**: 已修复，可独立运行
- ✅ **语法检查**: 所有脚本语法正确
- ✅ **模块检查**: 优雅处理模块不可用情况
- ⚠️ **绑定测试**: 需要生成并集成实际绑定代码
- ✅ **错误处理**: 提供详细的故障排除信息

测试脚本已经完全修复，可以在有或没有绑定代码的情况下优雅地运行！