# Lua Binding Generator 使用指南

## 基本用法

```bash
./lua_binding_generator [选项] <头文件1> [头文件2] ...
```

## 命令行选项

- `--output=<目录>` - 指定输出目录（默认：generated_bindings）
- `--module=<名称>` - 指定模块名称
- `--help` - 显示帮助信息
- `--version` - 显示版本信息

## 使用示例

### 生成简单绑定
```bash
# 为单个头文件生成绑定
./lua_binding_generator --output=output --module=MyModule my_class.h

# 为多个头文件生成绑定
./lua_binding_generator my_class1.h my_class2.h my_functions.h
```

### 支持的C++特性

工具支持以下C++特性的自动绑定：
- 类和结构体
- 成员函数和静态函数
- 构造函数和析构函数
- 属性和常量
- 枚举类型
- 命名空间
- 函数重载
- 运算符重载

## 绑定宏

使用以下宏标记需要导出的代码：

- `EXPORT_LUA_CLASS` - 导出类
- `EXPORT_LUA_FUNCTION` - 导出函数
- `EXPORT_LUA_ENUM` - 导出枚举
- `EXPORT_LUA_CONSTANT` - 导出常量
- 更多宏请参考framework文档

详细的宏使用说明请参考examples目录中的示例。