# Lua Binding Generator 构建指南

## 系统要求

- C++17编译器（GCC 7+, Clang 6+, MSVC 2019+）
- CMake 3.16+
- LLVM/Clang开发库（推荐18+）

## 构建步骤

### 1. 安装依赖

#### macOS (Homebrew)
```bash
brew install llvm cmake
```

#### Ubuntu/Debian
```bash
sudo apt-get install llvm-dev clang-dev cmake build-essential
```

### 2. 构建工具
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 3. 验证安装
```bash
./lua_binding_generator --help
```

## 构建选项

- `BUILD_EXAMPLES=ON` - 构建示例项目
- `GENERATE_LUA_BINDINGS=ON` - 自动生成绑定
- `USE_THIRDPARTY_LLVM=ON` - 使用内置LLVM

## 故障排除

常见问题和解决方案请参考项目根目录的README.md。