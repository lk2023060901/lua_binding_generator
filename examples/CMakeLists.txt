cmake_minimum_required(VERSION 3.16)
project(LuaBindingGenerator_Examples VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用 thirdparty 中的 Lua，不使用 find_package
# find_package(Lua REQUIRED) # 已禁用，使用 thirdparty 版本
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(SOL2 QUIET sol2)
endif()

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../generated_bindings
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LUA_INCLUDE_DIR}
)

# 如果找不到sol2，尝试包含系统路径，然后使用本地副本
if(NOT SOL2_FOUND)
    find_path(SOL2_INCLUDE_DIR sol/sol.hpp
        HINTS /usr/local/include /opt/homebrew/include /usr/include)
    if(SOL2_INCLUDE_DIR)
        include_directories(${SOL2_INCLUDE_DIR})
        message(STATUS "Found sol2 headers at: ${SOL2_INCLUDE_DIR}")
    else()
        # 使用本地thirdparty目录中的sol2
        set(LOCAL_SOL2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/sol2-3.3.0/include)
        if(EXISTS ${LOCAL_SOL2_DIR})
            include_directories(${LOCAL_SOL2_DIR})
            message(STATUS "Using local sol2 headers at: ${LOCAL_SOL2_DIR}")
        else()
            message(FATAL_ERROR "sol2 headers not found. Please install sol2 or ensure local copy is available.")
        endif()
    endif()
endif()

# 编译选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -O2)
    # 禁用Sol2相关的警告
    add_compile_options(-Wno-unknown-warning-option -Wno-unknown-pragmas)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /O2)
endif()

# ================================
# Complete Example (使用实际存在的文件)
# ================================

# 检查绑定文件是否存在
set(MODULE_BINDINGS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../generated_bindings/generated_module_bindings.cpp")
set(COMPLETE_EXAMPLE_SOURCES
    complete_example.h
    complete_example.cpp
    main.cpp
)

if(EXISTS "${MODULE_BINDINGS_FILE}")
    list(APPEND COMPLETE_EXAMPLE_SOURCES "../generated_bindings/generated_module_bindings.cpp")
    message(STATUS "Using generated module bindings for complete example")
else()
    message(WARNING "Module bindings not found - building without Lua integration")
endif()

add_executable(complete_example ${COMPLETE_EXAMPLE_SOURCES})

set_target_properties(complete_example PROPERTIES
    OUTPUT_NAME complete_example
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

# 为 complete_example 添加线程库和 Lua 库
find_package(Threads REQUIRED)
target_link_libraries(complete_example PRIVATE 
    Threads::Threads
    lua_static
    spdlog::spdlog
)

# ================================
# 其他示例目标（传统目标，但使用 complete_example 文件）
# ================================

# 为向后兼容性，创建别名目标
add_executable(comprehensive_test ${COMPLETE_EXAMPLE_SOURCES})
add_executable(simple_example ${COMPLETE_EXAMPLE_SOURCES})
add_executable(game_engine_example ${COMPLETE_EXAMPLE_SOURCES})

set_target_properties(comprehensive_test PROPERTIES
    OUTPUT_NAME comprehensive_test
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

set_target_properties(simple_example PROPERTIES
    OUTPUT_NAME simple_example
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

set_target_properties(game_engine_example PROPERTIES
    OUTPUT_NAME game_engine_example
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

# 为所有目标添加相同的依赖
target_link_libraries(comprehensive_test PRIVATE 
    Threads::Threads
    lua_static
    spdlog::spdlog
)

target_link_libraries(simple_example PRIVATE 
    Threads::Threads
    lua_static
    spdlog::spdlog
)

target_link_libraries(game_engine_example PRIVATE 
    Threads::Threads
    lua_static
    spdlog::spdlog
)

# ================================
# 安装配置
# ================================

install(TARGETS complete_example comprehensive_test simple_example game_engine_example
    RUNTIME DESTINATION examples
    COMPONENT examples
)

# 安装头文件（可选，用于参考）
install(FILES
    complete_example.h
    DESTINATION include/examples
    COMPONENT examples-dev
)

# ================================
# 测试配置（可选）
# ================================

enable_testing()

# 基础运行测试
add_test(NAME run_complete_example COMMAND complete_example)
add_test(NAME run_comprehensive_test COMMAND comprehensive_test)
add_test(NAME run_simple_example COMMAND simple_example)
add_test(NAME run_game_engine_example COMMAND game_engine_example)

# 如果有Lua测试脚本，可以添加Lua测试
# find_program(LUA_EXECUTABLE lua)
# if(LUA_EXECUTABLE)
#     add_test(NAME lua_binding_test COMMAND ${LUA_EXECUTABLE} scripts/test_all.lua)
# endif()

# ================================
# 自定义目标
# ================================

# 生成绑定的自定义目标
add_custom_target(generate_bindings
    COMMAND ${CMAKE_BINARY_DIR}/lua_binding_generator --verbose --output_dir=../generated_bindings complete_example.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS lua_binding_generator
    COMMENT "Generating Lua bindings for complete example"
)

# 运行所有示例的自定义目标
add_custom_target(run_all_examples
    COMMAND echo "Running Complete Example..." && $<TARGET_FILE:complete_example>
    DEPENDS complete_example
    COMMENT "Running complete example program"
)

# ================================
# 清理目标
# ================================

# 清理生成的绑定文件
add_custom_target(clean_bindings
    COMMAND ${CMAKE_COMMAND} -E remove_directory generated_bindings
    COMMAND ${CMAKE_COMMAND} -E remove_directory test_output
    COMMENT "Cleaning generated binding files"
)

# ================================
# 开发者信息
# ================================

message(STATUS "=== Lua Binding Generator Examples ===")
message(STATUS "Examples will be built to: ${CMAKE_BINARY_DIR}/examples")
message(STATUS "Available targets:")
message(STATUS "  - complete_example: 主要示例程序")
message(STATUS "  - comprehensive_test: 兼容性别名（同 complete_example）")
message(STATUS "  - simple_example: 兼容性别名（同 complete_example）")
message(STATUS "  - game_engine_example: 兼容性别名（同 complete_example）")
message(STATUS "  - generate_bindings: 生成 Lua 绑定文件")
message(STATUS "  - run_all_examples: 运行所有示例程序")
message(STATUS "  - clean_bindings: 清理生成的绑定文件")
message(STATUS "=======================================")

# 显示构建信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Build type: Debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Build type: Release")
else()
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")

# 提供使用说明
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../README.md
    ${CMAKE_BINARY_DIR}/examples/README.md
    COPYONLY
)