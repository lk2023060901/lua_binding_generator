cmake_minimum_required(VERSION 3.16)
project(lua_binding_generator VERSION 2.0.0 LANGUAGES CXX)

# C++ 标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 选项：是否自动生成 Lua 绑定文件
option(GENERATE_LUA_BINDINGS "自动生成 Lua 绑定文件" OFF)

# 选项：是否编译示例代码
option(BUILD_EXAMPLES "编译示例代码" OFF)

# 选项：是否使用 thirdparty 中的 LLVM（当系统找不到时）
option(USE_THIRDPARTY_LLVM "使用 thirdparty 中的 LLVM" OFF)

# 添加 thirdparty 库
add_subdirectory(thirdparty/lua-5.4.8)
add_subdirectory(thirdparty/spdlog-1.15.3)

# 注释：运行时库代码已整合到 src/framework 和 include/framework
# add_subdirectory(lib)

# 检测 LLVM/Clang - 按优先级顺序
set(LLVM_FOUND FALSE)
set(LLVM_ROOT "")
set(LLVM_INCLUDE_DIRS "")
set(LLVM_LIBRARY_DIRS "")

# 1. 尝试使用 find_package 查找系统安装的 LLVM
find_package(LLVM QUIET CONFIG)
if(LLVM_FOUND)
    set(LLVM_ROOT ${LLVM_INSTALL_PREFIX})
    set(LLVM_INCLUDE_DIRS ${LLVM_INCLUDE_DIRS})
    set(LLVM_LIBRARY_DIRS ${LLVM_LIBRARY_DIRS})
    message(STATUS "使用 find_package 找到的 LLVM: ${LLVM_ROOT}")
else()
    # 2. 检查环境变量
    if(DEFINED ENV{LLVM_ROOT} AND EXISTS "$ENV{LLVM_ROOT}")
        set(LLVM_ROOT "$ENV{LLVM_ROOT}")
        set(LLVM_INCLUDE_DIRS "${LLVM_ROOT}/include")
        set(LLVM_LIBRARY_DIRS "${LLVM_ROOT}/lib")
        set(LLVM_FOUND TRUE)
        message(STATUS "使用环境变量 LLVM_ROOT: ${LLVM_ROOT}")
    elseif(DEFINED ENV{CLANG_ROOT} AND EXISTS "$ENV{CLANG_ROOT}")
        set(LLVM_ROOT "$ENV{CLANG_ROOT}")
        set(LLVM_INCLUDE_DIRS "${LLVM_ROOT}/include")
        set(LLVM_LIBRARY_DIRS "${LLVM_ROOT}/lib")
        set(LLVM_FOUND TRUE)
        message(STATUS "使用环境变量 CLANG_ROOT: ${LLVM_ROOT}")
    else()
        # 3. 检查系统默认安装路径
        set(LLVM_SEARCH_PATHS
            "/opt/homebrew/opt/llvm"        # macOS Homebrew ARM
            "/usr/local/opt/llvm"           # macOS Homebrew Intel
            "/usr/lib/llvm-18"              # Ubuntu/Debian
            "/usr/lib/llvm-17"
            "/usr/lib/llvm-16"
            "/usr/lib/llvm-15"
            "/usr/local/llvm"               # 通用路径
            "/usr"                          # 系统路径
        )
        
        foreach(SEARCH_PATH ${LLVM_SEARCH_PATHS})
            if(EXISTS "${SEARCH_PATH}/include/clang" OR EXISTS "${SEARCH_PATH}/include/llvm")
                set(LLVM_ROOT "${SEARCH_PATH}")
                set(LLVM_INCLUDE_DIRS "${LLVM_ROOT}/include")
                set(LLVM_LIBRARY_DIRS "${LLVM_ROOT}/lib")
                set(LLVM_FOUND TRUE)
                message(STATUS "在系统路径找到 LLVM: ${LLVM_ROOT}")
                break()
            endif()
        endforeach()
        
        # 4. 如果系统找不到且启用了 USE_THIRDPARTY_LLVM，使用 thirdparty
        if(NOT LLVM_FOUND AND USE_THIRDPARTY_LLVM)
            set(THIRDPARTY_LLVM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/llvm-20.1.8")
            if(EXISTS "${THIRDPARTY_LLVM_ROOT}")
                set(LLVM_ROOT "${THIRDPARTY_LLVM_ROOT}")
                set(LLVM_INCLUDE_DIRS "${LLVM_ROOT}/include")
                set(LLVM_LIBRARY_DIRS "${LLVM_ROOT}/lib")
                set(LLVM_FOUND TRUE)
                message(STATUS "使用 thirdparty LLVM: ${LLVM_ROOT}")
                message(WARNING "使用 thirdparty LLVM 可能需要先编译。请参考 thirdparty/llvm-20.1.8/README.md")
            endif()
        endif()
        
        # 5. 都找不到则报错
        if(NOT LLVM_FOUND)
            message(FATAL_ERROR "未找到 LLVM/Clang 开发库。请：\n"
                              "1. 安装系统版本：brew install llvm 或 apt-get install llvm-dev clang-dev\n"
                              "2. 设置环境变量：export LLVM_ROOT=/path/to/llvm\n"
                              "3. 或启用 thirdparty：-DUSE_THIRDPARTY_LLVM=ON")
        endif()
    endif()
endif()

# 头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/include/framework
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/spdlog-1.15.3/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/sol2-3.3.0/include
    ${LLVM_INCLUDE_DIRS}
)

# 工具源文件列表 - 零配置版本
set(GENERATOR_SOURCES
    src/tools/compiler_detector.cpp
    src/tools/dynamic_compilation_database.cpp
    src/tools/smart_inference_engine.cpp
    src/tools/ast_visitor.cpp
    src/tools/direct_binding_generator.cpp
    src/tools/incremental_generator.cpp
    src/tools/logger.cpp
)

# 工具头文件列表 - 零配置版本
set(GENERATOR_HEADERS
    include/tools/compiler_detector.h
    include/tools/dynamic_compilation_database.h
    include/tools/smart_inference_engine.h
    include/tools/ast_visitor.h
    include/tools/direct_binding_generator.h
    include/tools/incremental_generator.h
    include/tools/logger.h
    include/framework/export_macros.h
)

# 创建可执行文件
add_executable(lua_binding_generator 
    src/tools/main.cpp
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

# 获取 LLVM 系统依赖（如果 llvm-config 存在）
set(LLVM_SYSTEM_LIBS "")
if(EXISTS "${LLVM_ROOT}/bin/llvm-config")
    execute_process(
        COMMAND ${LLVM_ROOT}/bin/llvm-config --system-libs
        OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
else()
    # 通用系统库
    set(LLVM_SYSTEM_LIBS "-lz -ltinfo -lm")
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(LLVM_SYSTEM_LIBS "${LLVM_SYSTEM_LIBS} -framework CoreFoundation")
    endif()
endif()

# 链接库
if(CLANG_CPP_LIB AND LLVM_LIB)
    # 使用共享库
    target_link_libraries(lua_binding_generator
        ${CLANG_CPP_LIB}
        ${LLVM_LIB}
        spdlog::spdlog
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
        spdlog::spdlog
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