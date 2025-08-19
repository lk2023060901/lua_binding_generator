/**
 * @file binding_integration_example.cpp
 * @brief 完整的绑定注册和Lua集成示例
 * 
 * 展示如何将生成的绑定与运行时库集成，实现完整的C++/Lua交互
 */

#include "lua_binding_runtime.h"
#include "../headers/macro_coverage.h"
#include "../headers/runtime_features.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

using namespace lua_runtime;
using namespace test_coverage;
using namespace runtime_test;

/**
 * @brief 绑定注册函数（通常由lua_binding_generator生成）
 * 
 * 这个函数展示了如何注册所有测试类的绑定
 */
bool registerCompleteTestBindings(sol::state& lua) {
    try {
        // 创建命名空间
        auto test_ns = lua["test_coverage"].get_or_create<sol::table>();
        auto runtime_ns = lua["runtime_test"].get_or_create<sol::table>();
        
        // 注册基础测试类
        test_ns.new_usertype<TestPlayer>("TestPlayer",
            sol::constructors<TestPlayer(const std::string&, int)>(),
            "getName", &TestPlayer::getName,
            "getLevel", &TestPlayer::getLevel,
            "setLevel", &TestPlayer::setLevel,
            "gainExperience", &TestPlayer::gainExperience,
            "isAlive", &TestPlayer::isAlive
        );
        
        test_ns.new_usertype<TestMathUtils>("TestMathUtils",
            "add", &TestMathUtils::add,
            "multiply", &TestMathUtils::multiply,
            "factorial", &TestMathUtils::factorial,
            "isPrime", &TestMathUtils::isPrime,
            "power", &TestMathUtils::power
        );
        
        test_ns.new_usertype<TestStringUtils>("TestStringUtils",
            "toUpper", &TestStringUtils::toUpper,
            "toLower", &TestStringUtils::toLower,
            "reverse", &TestStringUtils::reverse,
            "contains", &TestStringUtils::contains,
            "split", &TestStringUtils::split
        );
        
        test_ns.new_usertype<TestVector2D>("TestVector2D",
            sol::constructors<TestVector2D(), TestVector2D(double, double)>(),
            "x", &TestVector2D::x,
            "y", &TestVector2D::y,
            "length", &TestVector2D::length,
            "normalize", &TestVector2D::normalize,
            "dot", &TestVector2D::dot,
            sol::meta_function::addition, &TestVector2D::operator+,
            sol::meta_function::subtraction, &TestVector2D::operator-,
            sol::meta_function::multiplication, sol::resolve<TestVector2D(double) const>(&TestVector2D::operator*)
        );
        
        // 注册单例
        test_ns["GameManager"] = &TestGameManager::getInstance();
        
        // 注册枚举
        test_ns.new_enum<PlayerState>("PlayerState", {
            {"IDLE", PlayerState::IDLE},
            {"MOVING", PlayerState::MOVING},
            {"ATTACKING", PlayerState::ATTACKING},
            {"DEAD", PlayerState::DEAD}
        });
        
        test_ns.new_enum<LogLevel>("LogLevel", {
            {"DEBUG", LogLevel::DEBUG},
            {"INFO", LogLevel::INFO},
            {"WARNING", LogLevel::WARNING},
            {"ERROR", LogLevel::ERROR}
        });
        
        test_ns.new_enum<GameDifficulty>("GameDifficulty", {
            {"EASY", GameDifficulty::EASY},
            {"NORMAL", GameDifficulty::NORMAL},
            {"HARD", GameDifficulty::HARD}
        });
        
        // 注册常量
        test_ns["MAX_PLAYERS"] = MAX_PLAYERS;
        test_ns["DEFAULT_TIMEOUT"] = DEFAULT_TIMEOUT;
        test_ns["VERSION_STRING"] = VERSION_STRING;
        test_ns["PI_VALUE"] = PI_VALUE;
        test_ns["MAX_HEALTH"] = MAX_HEALTH;
        test_ns["GRAVITY"] = GRAVITY;
        test_ns["DEFAULT_NAME"] = DEFAULT_NAME;
        
        // 注册全局函数
        test_ns["initializeTestSystem"] = &initializeTestSystem;
        test_ns["cleanupTestSystem"] = &cleanupTestSystem;
        test_ns["validateTestData"] = &validateTestData;
        test_ns["generateRandomNumber"] = &generateRandomNumber;
        test_ns["processTestBatch"] = &processTestBatch;
        test_ns["formatTestOutput"] = &formatTestOutput;
        
        // 注册运行时测试类
        runtime_ns.new_usertype<TestTrackingAllocator>("TestTrackingAllocator",
            sol::constructors<TestTrackingAllocator()>(),
            "getTotalAllocated", &TestTrackingAllocator::getTotalAllocated,
            "getAllocationCount", &TestTrackingAllocator::getAllocationCount,
            "resetStats", &TestTrackingAllocator::resetStats,
            "getStatsReport", &TestTrackingAllocator::getStatsReport,
            "hasMemoryLeaks", &TestTrackingAllocator::hasMemoryLeaks
        );
        
        runtime_ns.new_usertype<ErrorHandlingTester>("ErrorHandlingTester",
            sol::constructors<ErrorHandlingTester()>(),
            "initializeRuntime", &ErrorHandlingTester::initializeRuntime,
            "testSuccessResult", &ErrorHandlingTester::testSuccessResult,
            "testErrorResult", &ErrorHandlingTester::testErrorResult,
            "testSyntaxError", &ErrorHandlingTester::testSyntaxError,
            "testRuntimeError", &ErrorHandlingTester::testRuntimeError,
            "getErrorCount", &ErrorHandlingTester::getErrorCount,
            "getLastErrorDetail", &ErrorHandlingTester::getLastErrorDetail
        );
        
        runtime_ns.new_usertype<PerformanceTester>("PerformanceTester",
            sol::constructors<PerformanceTester()>(),
            "initializeRuntime", &PerformanceTester::initializeRuntime,
            "benchmarkScriptExecution", &PerformanceTester::benchmarkScriptExecution,
            "benchmarkFunctionCall", &PerformanceTester::benchmarkFunctionCall,
            "benchmarkObjectCreation", &PerformanceTester::benchmarkObjectCreation,
            "getAverageExecutionTime", &PerformanceTester::getAverageExecutionTime,
            "getPerformanceReport", &PerformanceTester::getPerformanceReport
        );
        
        runtime_ns.new_usertype<HotReloadTester>("HotReloadTester",
            sol::constructors<HotReloadTester()>(),
            "testBasicReload", &HotReloadTester::testBasicReload,
            "testProtectedTableReload", &HotReloadTester::testProtectedTableReload,
            "testFileWatching", &HotReloadTester::testFileWatching,
            "getSuccessfulReloads", &HotReloadTester::getSuccessfulReloads,
            "getFailedReloads", &HotReloadTester::getFailedReloads,
            "getReloadReport", &HotReloadTester::getReloadReport
        );
        
        // 注册全局变量
        lua["global_test_verbose"] = &global_test_verbose;
        lua["global_test_seed"] = &global_test_seed;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "绑定注册失败: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 运行完整的集成测试示例
 */
int runIntegrationExample() {
    std::cout << "=== Lua绑定运行时库集成示例 ===" << std::endl;
    
    try {
        // 1. 创建带自定义分配器的运行时管理器
        auto tracking_allocator = std::make_shared<TestTrackingAllocator>();
        LuaRuntimeManager runtime(tracking_allocator);
        
        std::cout << "✅ 运行时管理器已创建（使用跟踪分配器）" << std::endl;
        
        // 2. 注册所有绑定
        if (!registerCompleteTestBindings(runtime.getLuaState())) {
            std::cerr << "❌ 绑定注册失败" << std::endl;
            return 1;
        }
        std::cout << "✅ 所有C++绑定已注册到Lua" << std::endl;
        
        // 3. 执行基础功能测试
        std::cout << "\n--- 基础功能测试 ---" << std::endl;
        
        auto result = runtime.executeScript(R"(
            -- 测试基础类功能
            local player = test_coverage.TestPlayer("Hero", 1)
            print("创建玩家: " .. player:getName() .. " (等级 " .. player:getLevel() .. ")")
            
            player:gainExperience(150)
            print("获得经验后等级: " .. player:getLevel())
            
            -- 测试数学工具
            local sum = test_coverage.TestMathUtils.add(10, 20)
            local factorial = test_coverage.TestMathUtils.factorial(5)
            print("数学测试: 10 + 20 = " .. sum .. ", 5! = " .. factorial)
            
            -- 测试向量运算
            local v1 = test_coverage.TestVector2D(3, 4)
            local v2 = test_coverage.TestVector2D(1, 1)
            local v3 = v1 + v2
            print("向量运算: (3,4) + (1,1) = (" .. v3.x .. "," .. v3.y .. ")")
            
            -- 测试单例
            test_coverage.GameManager:setDifficulty(test_coverage.GameDifficulty.HARD)
            print("游戏难度设置为: " .. tostring(test_coverage.GameManager:getDifficulty()))
            
            return "基础测试完成"
        )");
        
        if (result.isSuccess()) {
            std::cout << "✅ 基础功能测试通过" << std::endl;
        } else {
            std::cerr << "❌ 基础功能测试失败: " << result.error().toString() << std::endl;
        }
        
        // 4. 运行时功能集成测试
        std::cout << "\n--- 运行时功能集成测试 ---" << std::endl;
        
        auto runtime_result = runtime.executeScript(R"(
            -- 测试错误处理
            local error_tester = runtime_test.ErrorHandlingTester()
            error_tester:initializeRuntime()
            
            local success_test = error_tester:testSuccessResult()
            local error_test = error_tester:testErrorResult()
            
            print("错误处理测试 - 成功结果: " .. tostring(success_test))
            print("错误处理测试 - 错误结果: " .. tostring(error_test))
            print("错误计数: " .. error_tester:getErrorCount())
            
            -- 测试性能基准
            local perf_tester = runtime_test.PerformanceTester()
            perf_tester:initializeRuntime()
            
            local exec_time = perf_tester:benchmarkScriptExecution("return 42", 100)
            print("脚本执行基准测试: " .. exec_time .. "ms (100次迭代)")
            
            -- 测试热加载
            local reload_tester = runtime_test.HotReloadTester()
            local runtime_ptr = nil  -- 在实际使用中需要传递运行时指针
            -- reload_tester:initialize(runtime_ptr)
            
            print("运行时功能集成测试完成")
            return "运行时测试完成"
        )");
        
        if (runtime_result.isSuccess()) {
            std::cout << "✅ 运行时功能集成测试通过" << std::endl;
        } else {
            std::cerr << "❌ 运行时功能集成测试失败: " << runtime_result.error().toString() << std::endl;
        }
        
        // 5. 内存使用统计
        std::cout << "\n--- 内存使用统计 ---" << std::endl;
        std::cout << "总分配内存: " << tracking_allocator->getTotalAllocated() << " 字节" << std::endl;
        std::cout << "分配次数: " << tracking_allocator->getAllocationCount() << std::endl;
        std::cout << "内存泄漏检查: " << (tracking_allocator->hasMemoryLeaks() ? "有泄漏" : "无泄漏") << std::endl;
        
        // 6. 高级分配器演示
        std::cout << "\n--- 高级分配器演示 ---" << std::endl;
        
        // 堆栈分配器演示
        auto stack_allocator = AllocatorFactory::createStackAllocator(1024);
        std::cout << "✅ 堆栈分配器已创建 (1KB)" << std::endl;
        
        // 内存池分配器演示
        auto pool_allocator = AllocatorFactory::createPoolAllocator(64, 100);
        std::cout << "✅ 内存池分配器已创建 (64字节×100块)" << std::endl;
        
        // 虚拟内存分配器演示
        auto vm_allocator = AllocatorFactory::createVirtualMemoryAllocator();
        std::cout << "✅ 虚拟内存分配器已创建 (页大小: " << vm_allocator->getPageSize() << " 字节)" << std::endl;
        
        std::cout << "\n=== 集成示例完成 ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 集成示例运行失败: " << e.what() << std::endl;
        return 1;
    }
}

/**
 * @brief 主函数 - 可以作为独立程序运行
 */
int main() {
    return runIntegrationExample();
}