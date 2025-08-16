cmake_minimum_required(VERSION 3.16)
project(lua_binding_generator VERSION 2.0.0 LANGUAGES CXX)

# C++ 标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 选项：是否自动生成 Lua 绑定文件
option(GENERATE_LUA_BINDINGS "自动生成 Lua 绑定文件" OFF)

# 选项：是否编译示例代码
option(BUILD_EXAMPLES "编译示例代码" OFF)

# 检测 LLVM 安装
if(EXISTS "/opt/homebrew/opt/llvm")
    # macOS Homebrew LLVM
    set(LLVM_ROOT "/opt/homebrew/opt/llvm")
    set(LLVM_INCLUDE_DIRS "${LLVM_ROOT}/include")
    set(LLVM_LIBRARY_DIRS "${LLVM_ROOT}/lib")
    message(STATUS "使用 Homebrew LLVM: ${LLVM_ROOT}")
elseif(EXISTS "/usr/local/opt/llvm")
    # macOS Homebrew LLVM (Intel)
    set(LLVM_ROOT "/usr/local/opt/llvm")
    set(LLVM_INCLUDE_DIRS "${LLVM_ROOT}/include")
    set(LLVM_LIBRARY_DIRS "${LLVM_ROOT}/lib")
    message(STATUS "使用 Homebrew LLVM (Intel): ${LLVM_ROOT}")
elseif(EXISTS "/usr/include/clang")
    # 系统安装的 LLVM/Clang
    set(LLVM_INCLUDE_DIRS "/usr/include")
    set(LLVM_LIBRARY_DIRS "/usr/lib")
    message(STATUS "使用系统 LLVM/Clang")
else()
    message(FATAL_ERROR "未找到 LLVM/Clang 开发库。请安装：brew install llvm")
endif()

# 头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/spdlog-1.15.3/include
    ${LLVM_INCLUDE_DIRS}
)

# 源文件列表 - 零配置版本
set(GENERATOR_SOURCES
    src/compiler_detector.cpp
    src/dynamic_compilation_database.cpp
    src/smart_inference_engine.cpp
    src/ast_visitor.cpp
    src/direct_binding_generator.cpp
    src/incremental_generator.cpp
    src/logger.cpp
)

# 头文件列表 - 零配置版本
set(GENERATOR_HEADERS
    include/compiler_detector.h
    include/dynamic_compilation_database.h
    include/smart_inference_engine.h
    include/ast_visitor.h
    include/direct_binding_generator.h
    include/incremental_generator.h
    include/export_macros.h
    include/logger.h
)

# 创建可执行文件
add_executable(lua_binding_generator 
    main.cpp
    ${GENERATOR_SOURCES}
    ${GENERATOR_HEADERS}
)

# 设置目标属性
set_target_properties(lua_binding_generator PROPERTIES
    OUTPUT_NAME lua_binding_generator
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 链接库目录
link_directories(${LLVM_LIBRARY_DIRS})

# 查找必要的 Clang/LLVM 库
find_library(CLANG_TOOLING_LIB clangTooling PATHS ${LLVM_LIBRARY_DIRS})
find_library(CLANG_FRONTEND_LIB clangFrontend PATHS ${LLVM_LIBRARY_DIRS})
find_library(CLANG_AST_LIB clangAST PATHS ${LLVM_LIBRARY_DIRS})
find_library(CLANG_BASIC_LIB clangBasic PATHS ${LLVM_LIBRARY_DIRS})
find_library(CLANG_LEX_LIB clangLex PATHS ${LLVM_LIBRARY_DIRS})
find_library(CLANG_PARSE_LIB clangParse PATHS ${LLVM_LIBRARY_DIRS})
find_library(CLANG_SEMA_LIB clangSema PATHS ${LLVM_LIBRARY_DIRS})
find_library(CLANG_EDIT_LIB clangEdit PATHS ${LLVM_LIBRARY_DIRS})
find_library(CLANG_ANALYSIS_LIB clangAnalysis PATHS ${LLVM_LIBRARY_DIRS})
find_library(CLANG_DRIVER_LIB clangDriver PATHS ${LLVM_LIBRARY_DIRS})
find_library(CLANG_SERIALIZATION_LIB clangSerialization PATHS ${LLVM_LIBRARY_DIRS})

# 使用 clang-cpp 共享库而不是单独的库
find_library(CLANG_CPP_LIB clang-cpp PATHS ${LLVM_LIBRARY_DIRS})
find_library(LLVM_LIB LLVM PATHS ${LLVM_LIBRARY_DIRS})

# 获取 LLVM 系统依赖
execute_process(
    COMMAND ${LLVM_ROOT}/bin/llvm-config --system-libs
    OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# 链接库
if(CLANG_CPP_LIB AND LLVM_LIB)
    # 使用共享库
    target_link_libraries(lua_binding_generator
        ${CLANG_CPP_LIB}
        ${LLVM_LIB}
        ${LLVM_SYSTEM_LIBS}
    )
    message(STATUS "使用共享库: clang-cpp + LLVM")
else()
    # 回退到单独的静态库
    target_link_libraries(lua_binding_generator
        ${CLANG_TOOLING_LIB}
        ${CLANG_FRONTEND_LIB}
        ${CLANG_AST_LIB}
        ${CLANG_BASIC_LIB}
        ${CLANG_LEX_LIB}
        ${CLANG_PARSE_LIB}
        ${CLANG_SEMA_LIB}
        ${CLANG_EDIT_LIB}
        ${CLANG_ANALYSIS_LIB}
        ${CLANG_DRIVER_LIB}
        ${CLANG_SERIALIZATION_LIB}
        ${LLVM_SYSTEM_LIBS}
    )
    message(STATUS "使用静态库")
endif()

# 编译选项
target_compile_options(lua_binding_generator PRIVATE
    -fno-rtti
)

# 显示配置信息
message(STATUS "=== 零配置 Lua Binding Generator ===")
message(STATUS "C++ 标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "LLVM 根目录: ${LLVM_ROOT}")
message(STATUS "LLVM 包含目录: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM 库目录: ${LLVM_LIBRARY_DIRS}")
if(CLANG_TOOLING_LIB)
    message(STATUS "Clang Tooling 库: ${CLANG_TOOLING_LIB}")
else()
    message(WARNING "未找到 Clang Tooling 库")
endif()
message(STATUS "====================================")

# 安装配置
install(TARGETS lua_binding_generator
    RUNTIME DESTINATION bin
    COMPONENT tools
)

# Lua 绑定生成逻辑
if(GENERATE_LUA_BINDINGS)
    message(STATUS "启用 Lua 绑定自动生成")
    
    # 平台检测
    if(WIN32)
        # Windows 平台
        set(BINDING_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/generate_bindings.bat")
        set(SCRIPT_COMMAND cmd /c "${BINDING_SCRIPT}")
    else()
        # Unix-like 平台 (Linux, macOS, etc.)
        set(BINDING_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/generate_bindings.sh")
        set(SCRIPT_COMMAND "${BINDING_SCRIPT}")
    endif()
    
    # 检查脚本是否存在
    if(EXISTS "${BINDING_SCRIPT}")
        message(STATUS "绑定生成脚本: ${BINDING_SCRIPT}")
        
        # 添加自定义目标来生成绑定
        add_custom_target(generate_lua_bindings
            COMMAND ${SCRIPT_COMMAND} -c -o generated_bindings
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            DEPENDS lua_binding_generator
            COMMENT "生成 Lua 绑定文件"
            VERBATIM
        )
        
        # 添加自定义目标来运行绑定测试
        add_custom_target(test_lua_bindings
            COMMAND ${SCRIPT_COMMAND} -c -t -o test_bindings
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            DEPENDS lua_binding_generator
            COMMENT "测试 Lua 绑定生成"
            VERBATIM
        )
        
        # 将绑定生成添加到默认构建目标
        add_dependencies(lua_binding_generator generate_lua_bindings)
        
        message(STATUS "可用的自定义目标:")
        message(STATUS "  - generate_lua_bindings: 生成 Lua 绑定文件")
        message(STATUS "  - test_lua_bindings: 测试绑定生成")
    else()
        message(WARNING "绑定生成脚本不存在: ${BINDING_SCRIPT}")
        message(WARNING "请确保 scripts/ 目录中包含正确的脚本文件")
    endif()
else()
    message(STATUS "Lua 绑定自动生成已禁用 (使用 -DGENERATE_LUA_BINDINGS=ON 启用)")
endif()

# 示例项目编译配置
if(BUILD_EXAMPLES)
    message(STATUS "启用示例项目编译")
    
    # 检查 examples 目录是否存在
    if(EXISTS "${CMAKE_SOURCE_DIR}/examples")
        message(STATUS "示例项目目录: ${CMAKE_SOURCE_DIR}/examples")
        
        # 包含示例项目
        add_subdirectory(examples)
        
        message(STATUS "可用的示例项目:")
        message(STATUS "  - simple_example: 基础绑定示例")
        message(STATUS "  - game_engine_example: 游戏引擎示例")
        message(STATUS "  - comprehensive_test: 综合特性测试")
    else()
        message(WARNING "示例项目目录不存在: ${CMAKE_SOURCE_DIR}/examples")
        message(WARNING "请确保 examples/ 目录存在并包含示例项目")
    endif()
else()
    message(STATUS "示例项目编译已禁用 (使用 -DBUILD_EXAMPLES=ON 启用)")
endif()