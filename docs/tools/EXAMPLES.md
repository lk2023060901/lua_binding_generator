# Lua Binding Generator 示例代码

## 基础示例

### 简单类绑定

C++ 头文件 (Calculator.h):
```cpp
#include "export_macros.h"

EXPORT_LUA_CLASS()
class Calculator {
public:
    EXPORT_LUA_CONSTRUCTOR()
    Calculator() = default;
    
    EXPORT_LUA_FUNCTION()
    int add(int a, int b) { return a + b; }
    
    EXPORT_LUA_FUNCTION()
    int multiply(int a, int b) { return a * b; }
    
    EXPORT_LUA_PROPERTY()
    double result = 0.0;
};
```

生成绑定:
```bash
./lua_binding_generator --module=Calculator Calculator.h
```

Lua 使用:
```lua
local calc = Calculator.new()
local sum = calc:add(10, 20)
print("10 + 20 = " .. sum)
```

### 枚举绑定

C++ 代码:
```cpp
EXPORT_LUA_ENUM()
enum class Status {
    EXPORT_LUA_ENUM_VALUE() Success = 0,
    EXPORT_LUA_ENUM_VALUE() Error = 1,
    EXPORT_LUA_ENUM_VALUE() Pending = 2
};
```

Lua 使用:
```lua
local status = Status.Success
if status == Status.Success then
    print("操作成功")
end
```

### 命名空间绑定

C++ 代码:
```cpp
EXPORT_LUA_NAMESPACE()
namespace MathUtils {
    EXPORT_LUA_FUNCTION()
    double PI() { return 3.14159; }
    
    EXPORT_LUA_FUNCTION()
    double square(double x) { return x * x; }
}
```

Lua 使用:
```lua
local pi = MathUtils.PI()
local result = MathUtils.square(5.0)
```

## 高级示例

请参考 examples/ 目录中的完整示例项目。