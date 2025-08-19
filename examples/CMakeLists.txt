cmake_minimum_required(VERSION 3.16)
project(LuaBindingGenerator_Examples VERSION 2.0.0 LANGUAGES CXX)

# C++ 标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 项目选项
option(BUILD_COMPLETE_TEST "构建完整的宏覆盖和运行时集成测试" ON)
option(BUILD_RUNTIME_DEMOS "构建运行时框架演示" OFF)

# 显示配置信息
message(STATUS "=== Lua Binding Generator Examples v2.0.0 ===")
message(STATUS "构建完整测试项目: ${BUILD_COMPLETE_TEST}")
message(STATUS "构建运行时演示: ${BUILD_RUNTIME_DEMOS}")

# 通用设置
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -O2)
    # 禁用Sol2相关的警告
    add_compile_options(-Wno-unknown-warning-option -Wno-unknown-pragmas)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /O2)
endif()

# 查找依赖库
find_package(Threads REQUIRED)

# 查找Sol2
find_path(SOL2_INCLUDE_DIR sol/sol.hpp
    HINTS 
        ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/sol2-3.3.0/include
        /usr/local/include 
        /opt/homebrew/include 
        /usr/include
    DOC "Sol2 include directory"
)

if(NOT SOL2_INCLUDE_DIR)
    message(FATAL_ERROR "Sol2 headers not found. Please install sol2 or ensure it's available in thirdparty/")
endif()

message(STATUS "Found Sol2 headers: ${SOL2_INCLUDE_DIR}")

# 包含通用目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/framework
    ${CMAKE_CURRENT_SOURCE_DIR}/../generated_bindings
    ${SOL2_INCLUDE_DIR}
)

# 添加子项目
if(BUILD_COMPLETE_TEST)
    message(STATUS "添加完整测试子项目")
    add_subdirectory(complete_test)
else()
    message(STATUS "跳过完整测试子项目")
endif()

if(BUILD_RUNTIME_DEMOS)
    message(STATUS "添加运行时框架演示")
    add_subdirectory(basic_runtime_usage)
    add_subdirectory(hot_reload_demo)
    add_subdirectory(memory_allocator_demo)
else()
    message(STATUS "跳过运行时框架演示")
endif()

# 全局测试目标
if(BUILD_COMPLETE_TEST)
    # 添加运行所有测试的目标
    add_custom_target(run_all_tests
        COMMAND echo "运行完整测试套件..." && $<TARGET_FILE:complete_test>
        DEPENDS complete_test
        COMMENT "运行所有测试项目"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    # 添加生成绑定的目标
    add_custom_target(generate_test_bindings
        COMMAND echo "生成测试绑定..." && 
                ${CMAKE_BINARY_DIR}/lua_binding_generator 
                --output=${CMAKE_CURRENT_SOURCE_DIR}/../generated_bindings
                ${CMAKE_CURRENT_SOURCE_DIR}/complete_test/headers/macro_coverage.h
                ${CMAKE_CURRENT_SOURCE_DIR}/complete_test/headers/runtime_features.h
        DEPENDS lua_binding_generator
        COMMENT "为测试项目生成Lua绑定"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

# 安装配置
if(BUILD_COMPLETE_TEST)
    install(TARGETS complete_test
        RUNTIME DESTINATION examples
        COMPONENT examples
    )
endif()

# 显示构建信息
message(STATUS "C++ 标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "编译器: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "Sol2 路径: ${SOL2_INCLUDE_DIR}")
message(STATUS "===============================================")