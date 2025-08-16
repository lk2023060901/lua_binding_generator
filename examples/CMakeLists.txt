cmake_minimum_required(VERSION 3.16)
project(LuaBindingGenerator_Examples VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(Lua REQUIRED)
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
# Comprehensive Test Example
# ================================

# Check if binding files exist
set(MODULE_BINDINGS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../generated_bindings/generated_module_bindings.cpp")
set(COMPREHENSIVE_SOURCES
    comprehensive_test.h
    comprehensive_test.cpp
    comprehensive_main.cpp
)

if(EXISTS "${MODULE_BINDINGS_FILE}")
    list(APPEND COMPREHENSIVE_SOURCES "../generated_bindings/generated_module_bindings.cpp")
    message(STATUS "Using generated module bindings")
else()
    message(WARNING "Module bindings not found - building without Lua integration")
endif()

add_executable(comprehensive_test ${COMPREHENSIVE_SOURCES})

set_target_properties(comprehensive_test PROPERTIES
    OUTPUT_NAME comprehensive_test
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

# 为comprehensive_test添加线程库和Lua库
find_package(Threads REQUIRED)
target_link_libraries(comprehensive_test PRIVATE 
    Threads::Threads
    ${LUA_LIBRARIES}
)

# ================================
# Simple Example
# ================================

# Check if binding files exist  
set(SIMPLE_SOURCES
    simple_example.h
    simple_example.cpp
    simple_main.cpp
)

if(EXISTS "${MODULE_BINDINGS_FILE}")
    list(APPEND SIMPLE_SOURCES "../generated_bindings/generated_module_bindings.cpp")
    message(STATUS "Using generated module bindings for simple example")
else()
    message(WARNING "Module bindings not found - building without Lua integration")
endif()

add_executable(simple_example ${SIMPLE_SOURCES})

set_target_properties(simple_example PROPERTIES
    OUTPUT_NAME simple_example
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

# 为simple_example添加Lua库
target_link_libraries(simple_example PRIVATE ${LUA_LIBRARIES})

# ================================
# Game Engine Example
# ================================

# Check if binding files exist
set(ENGINE_SOURCES
    game_engine.h
    game_engine.cpp
    game_engine_main.cpp
)

if(EXISTS "${MODULE_BINDINGS_FILE}")
    list(APPEND ENGINE_SOURCES "../generated_bindings/generated_module_bindings.cpp")
    message(STATUS "Using generated module bindings for game engine example")
else()
    message(WARNING "Module bindings not found - building without Lua integration")
endif()

add_executable(game_engine_example ${ENGINE_SOURCES})

set_target_properties(game_engine_example PROPERTIES
    OUTPUT_NAME game_engine_example
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

# 为game_engine_example添加线程库和Lua库
find_package(Threads REQUIRED)
target_link_libraries(game_engine_example PRIVATE 
    Threads::Threads
    ${LUA_LIBRARIES}
)

# ================================
# 安装配置
# ================================

install(TARGETS comprehensive_test simple_example game_engine_example
    RUNTIME DESTINATION examples
    COMPONENT examples
)

# 安装头文件（可选，用于参考）
install(FILES
    comprehensive_test.h
    simple_example.h
    game_engine.h
    DESTINATION include/examples
    COMPONENT examples-dev
)

# ================================
# 测试配置（可选）
# ================================

enable_testing()

# 基础运行测试
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
    COMMAND ${CMAKE_SOURCE_DIR}/lua_binding_generator --verbose --output_dir=generated_bindings examples/*.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating Lua bindings for all examples"
)

# 运行所有示例的自定义目标
add_custom_target(run_all_examples
    COMMAND echo "Running Comprehensive Test..." && $<TARGET_FILE:comprehensive_test>
    COMMAND echo "Running Simple Example..." && $<TARGET_FILE:simple_example>
    COMMAND echo "Running Game Engine Example..." && $<TARGET_FILE:game_engine_example>
    DEPENDS comprehensive_test simple_example game_engine_example
    COMMENT "Running all example programs"
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
message(STATUS "  - comprehensive_test: Complete feature demonstration")
message(STATUS "  - simple_example: Basic usage example")
message(STATUS "  - game_engine_example: Advanced game engine simulation")
message(STATUS "  - generate_bindings: Generate Lua bindings for examples")
message(STATUS "  - run_all_examples: Run all example programs")
message(STATUS "  - clean_bindings: Clean generated binding files")
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