/**
 * @file runtime_features.h
 * @brief 运行时库集成测试类
 * 
 * 这个文件专门用于测试lua_binding_runtime库与生成绑定的集成功能，
 * 包括热加载、内存管理、错误处理等高级功能。
 * 
 * 测试功能：
 * 1. LuaRuntimeManager基础功能
 * 2. 脚本热加载系统
 * 3. 自定义内存分配器
 * 4. 错误处理和Result类型
 * 5. 文件监控和回调
 * 6. 性能测试和压力测试
 * 7. 多线程安全性（基础）
 */

#pragma once

#include "export_macros.h"
#include "lua_binding_runtime.h"
#include "macro_coverage.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <atomic>
#include <thread>
#include <fstream>

// 使用实际的运行时库

using namespace lua_runtime;

// ================================
// 运行时集成测试模块
// ================================

EXPORT_LUA_MODULE(RuntimeIntegrationTest)

namespace runtime_test {

// ================================
// 测试用内存分配器
// ================================

/**
 * @brief 测试用的跟踪内存分配器
 * 记录所有内存分配和释放操作，用于测试内存管理
 */
class EXPORT_LUA_CLASS() TestTrackingAllocator : public MemoryAllocator {
public:
    TestTrackingAllocator();
    ~TestTrackingAllocator() override;
    
    // MemoryAllocator接口实现
    void* allocate(size_t size, size_t alignment = sizeof(void*)) override;
    void deallocate(void* ptr, size_t size) override;
    void* reallocate(void* ptr, size_t old_size, size_t new_size) override;
    
    // 统计信息
    EXPORT_LUA_PROPERTY(access=readonly)
    size_t getTotalAllocated() const override;
    
    EXPORT_LUA_PROPERTY(access=readonly)
    size_t getPeakAllocated() const;
    
    EXPORT_LUA_PROPERTY(access=readonly)
    size_t getAllocationCount() const override;
    
    EXPORT_LUA_PROPERTY(access=readonly)
    size_t getDeallocationCount() const;
    
    EXPORT_LUA_PROPERTY(access=readonly)
    size_t getActiveAllocations() const;
    
    // 统计重置
    void resetStats();
    std::string getStatsReport() const;
    
    // 泄漏检测
    bool hasMemoryLeaks() const;
    std::vector<std::string> getLeakReport() const;

private:
    mutable std::mutex mutex_;
    size_t total_allocated_ = 0;
    size_t peak_allocated_ = 0;
    size_t allocation_count_ = 0;
    size_t deallocation_count_ = 0;
    std::unordered_map<void*, size_t> active_allocations_;
    std::vector<std::string> allocation_history_;
};

/**
 * @brief 内存池分配器（简化版本）
 * 用于测试高性能内存分配场景
 */
class EXPORT_LUA_CLASS() TestPoolAllocator : public MemoryAllocator {
public:
    TestPoolAllocator(size_t pool_size = 1024 * 1024, size_t block_size = 64);
    ~TestPoolAllocator() override;
    
    // MemoryAllocator接口实现
    void* allocate(size_t size, size_t alignment = sizeof(void*)) override;
    void deallocate(void* ptr, size_t size) override;
    void* reallocate(void* ptr, size_t old_size, size_t new_size) override;
    
    // 内存池统计
    EXPORT_LUA_PROPERTY(access=readonly)
    size_t getPoolSize() const;
    
    EXPORT_LUA_PROPERTY(access=readonly)
    size_t getBlockSize() const;
    
    EXPORT_LUA_PROPERTY(access=readonly)
    size_t getUsedBlocks() const;
    
    EXPORT_LUA_PROPERTY(access=readonly)
    size_t getAvailableBlocks() const;
    
    EXPORT_LUA_PROPERTY(access=readonly)
    double getFragmentation() const;
    
    // 内存池管理
    void reset();
    bool isFull() const;
    std::string getPoolInfo() const;

private:
    void initializePool();
    
    void* pool_memory_;
    size_t pool_size_;
    size_t block_size_;
    size_t block_count_;
    void* free_list_;
    size_t used_blocks_;
    mutable std::mutex mutex_;
};

// ================================
// 运行时管理器测试助手
// ================================

/**
 * @brief 运行时测试助手类
 * 封装常用的测试操作和验证逻辑
 */
class EXPORT_LUA_CLASS() RuntimeTestHelper {
public:
    RuntimeTestHelper();
    ~RuntimeTestHelper();
    
    // 基础运行时操作
    bool initializeRuntime();
    bool initializeWithAllocator(std::shared_ptr<MemoryAllocator> allocator);
    void shutdownRuntime();
    
    // 绑定注册测试
    bool registerTestBindings();
    bool testBasicBindings();
    bool testComplexBindings();
    
    // 脚本执行测试
    bool executeSimpleScript(const std::string& script);
    bool executeScriptFile(const std::string& filename);
    std::string getLastExecutionResult() const;
    std::string getLastError() const;
    
    // 热加载测试
    bool setupHotReload();
    bool registerHotReloadScript(const std::string& name, const std::string& filepath);
    bool testScriptReload(const std::string& name);
    std::vector<std::string> getHotReloadHistory() const;
    
    // 性能测试
    double measureExecutionTime(const std::string& script);
    double measureFunctionCall(const std::string& function_name, int iterations = 1000);
    std::string getPerformanceReport() const;
    
    // 压力测试
    bool runStressTest(int object_count = 1000, int iteration_count = 100);
    bool runMemoryStressTest(size_t memory_size = 10 * 1024 * 1024);
    bool runConcurrencyTest(int thread_count = 4);
    
    // 状态查询
    EXPORT_LUA_PROPERTY(access=readonly)
    bool isRuntimeActive() const;
    
    std::string getRuntimeInfo() const;
    std::string getMemoryInfo() const;

private:
    std::unique_ptr<LuaRuntimeManager> runtime_;
    std::shared_ptr<MemoryAllocator> allocator_;
    std::string last_result_;
    std::string last_error_;
    std::vector<std::string> hot_reload_history_;
    std::vector<double> performance_history_;
    mutable std::mutex test_mutex_;
};

// ================================
// 热加载测试类
// ================================

/**
 * @brief 热加载功能专门测试类
 */
class EXPORT_LUA_CLASS() HotReloadTester {
public:
    HotReloadTester();
    ~HotReloadTester();
    
    // 热加载设置
    bool initialize(std::shared_ptr<LuaRuntimeManager> runtime);
    void setupProtectedTables();
    void setupCallbacks();
    
    // 脚本管理
    bool createTestScript(const std::string& name, const std::string& content);
    bool updateTestScript(const std::string& name, const std::string& new_content);
    bool deleteTestScript(const std::string& name);
    
    // 热加载测试
    bool testBasicReload();
    bool testProtectedTableReload();
    bool testCallbackReload();
    bool testErrorRecovery();
    bool testConcurrentReload();
    
    // 状态保护测试
    bool testStateProtection();
    bool testDataPersistence();
    bool testPartialReload();
    
    // 文件监控测试
    bool testFileWatching();
    bool testAutoReload();
    
    // 统计和报告
    EXPORT_LUA_PROPERTY(access=readonly)
    int getSuccessfulReloads() const;
    
    EXPORT_LUA_PROPERTY(access=readonly)
    int getFailedReloads() const;
    
    std::vector<std::string> getReloadLog() const;
    std::string getReloadReport() const;

private:
    void onPreReload(const HotReloadEvent& event);
    void onPostReload(const HotReloadEvent& event);
    
    std::shared_ptr<LuaRuntimeManager> runtime_;
    std::string test_script_dir_;
    std::vector<std::string> reload_log_;
    int successful_reloads_ = 0;
    int failed_reloads_ = 0;
    mutable std::mutex reload_mutex_;
};

// ================================
// 错误处理测试类
// ================================

/**
 * @brief 错误处理和Result类型测试
 */
class EXPORT_LUA_CLASS() ErrorHandlingTester {
public:
    ErrorHandlingTester();
    ~ErrorHandlingTester();
    
    // 初始化运行时环境
    bool initializeRuntime();
    
    // Result类型测试
    bool testSuccessResult();
    bool testErrorResult();
    bool testResultChaining();
    bool testResultConversion();
    
    // 脚本错误处理
    bool testSyntaxError();
    bool testRuntimeError();
    bool testTypeError();
    bool testMemoryError();
    
    // 异常恢复测试
    bool testErrorRecovery();
    bool testStateRollback();
    bool testContinuousOperation();
    
    // 错误报告
    EXPORT_LUA_PROPERTY(access=readonly)
    int getErrorCount() const;
    
    std::vector<std::string> getErrorHistory() const;
    std::string getLastErrorDetail() const;
    
    // 错误模拟
    void simulateMemoryError();
    void simulateLuaError();
    void simulateFileError();

private:
    std::vector<std::string> error_history_;
    int error_count_ = 0;
    std::string last_error_detail_;
    std::unique_ptr<LuaRuntimeManager> runtime_;
    mutable std::mutex error_mutex_;
};

// ================================
// 性能基准测试类
// ================================

/**
 * @brief 性能基准测试和分析
 */
class EXPORT_LUA_CLASS() PerformanceTester {
public:
    PerformanceTester();
    ~PerformanceTester();
    
    // 初始化性能测试环境
    bool initializeRuntime();
    bool initializeWithAllocator(std::shared_ptr<MemoryAllocator> allocator);
    void cleanup();
    
    // 访问运行时
    LuaRuntimeManager& getRuntime();
    
    // 基准测试
    double benchmarkScriptExecution(const std::string& script, int iterations = 1000);
    double benchmarkFunctionCall(const std::string& function_name, int iterations = 1000);
    double benchmarkObjectCreation(const std::string& class_name, int count = 1000);
    double benchmarkMemoryAllocation(size_t size, int iterations = 1000);
    
    // 比较测试
    double compareAllocators(MemoryAllocator* allocator1, MemoryAllocator* allocator2, int iterations = 1000);
    double compareExecutionMethods(const std::string& script1, const std::string& script2, int iterations = 100);
    
    // 负载测试
    bool runLoadTest(int concurrent_scripts = 10, int duration_seconds = 30);
    bool runMemoryLoadTest(size_t max_memory = 100 * 1024 * 1024);
    bool runCPULoadTest(int thread_count = 4, int duration_seconds = 10);
    
    // 统计和分析
    EXPORT_LUA_PROPERTY(access=readonly)
    double getAverageExecutionTime() const;
    
    EXPORT_LUA_PROPERTY(access=readonly)
    double getPeakMemoryUsage() const;
    
    std::string getPerformanceReport() const;
    std::vector<double> getTimeHistory() const;
    void clearHistory();
    
    // 性能阈值检查
    bool checkPerformanceThreshold(double max_time_ms = 100.0);
    bool checkMemoryThreshold(size_t max_memory_mb = 100);

private:
    struct BenchmarkResult {
        double min_time;
        double max_time;
        double avg_time;
        double total_time;
        int iterations;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    std::vector<BenchmarkResult> benchmark_history_;
    std::vector<double> execution_times_;
    double peak_memory_usage_ = 0.0;
    mutable std::recursive_mutex perf_mutex_;
    
    // 运行时管理器
    std::unique_ptr<LuaRuntimeManager> runtime_;
    std::shared_ptr<MemoryAllocator> allocator_;
    
    BenchmarkResult runBenchmark(std::function<void()> test_func, int iterations);
    void updateMemoryUsage();
};

// ================================
// 集成测试协调器
// ================================

/**
 * @brief 集成测试协调器
 * 统一管理所有运行时测试，提供完整的测试套件
 */
class EXPORT_LUA_SINGLETON() IntegrationTestCoordinator {
public:
    static IntegrationTestCoordinator& getInstance();
    
    // 测试套件管理
    bool initializeTestSuite();
    void shutdownTestSuite();
    
    // 测试执行
    bool runAllTests();
    bool runBasicTests();
    bool runAdvancedTests();
    bool runPerformanceTests();
    bool runStressTests();
    
    // 单独测试类别
    bool runMacroTests();
    bool runRuntimeTests();
    bool runHotReloadTests();
    bool runErrorHandlingTests();
    bool runMemoryTests();
    
    // 测试配置
    void setTestConfiguration(const std::string& config_json);
    std::string getTestConfiguration() const;
    void setVerboseOutput(bool enabled);
    void setTestTimeout(int timeout_seconds);
    
    // 结果和报告
    EXPORT_LUA_PROPERTY(access=readonly)
    int getTotalTests() const;
    
    EXPORT_LUA_PROPERTY(access=readonly)
    int getPassedTests() const;
    
    EXPORT_LUA_PROPERTY(access=readonly)
    int getFailedTests() const;
    
    EXPORT_LUA_PROPERTY(access=readonly)
    double getTestDuration() const;
    
    std::string getTestReport() const;
    std::string getFailureReport() const;
    std::vector<std::string> getTestLog() const;
    
    // 测试数据管理
    void clearTestData();
    void exportTestResults(const std::string& filename);
    bool importTestExpectations(const std::string& filename);

private:
    IntegrationTestCoordinator() = default;
    ~IntegrationTestCoordinator() = default;
    IntegrationTestCoordinator(const IntegrationTestCoordinator&) = delete;
    IntegrationTestCoordinator& operator=(const IntegrationTestCoordinator&) = delete;
    
    bool runTestCategory(const std::string& category_name, std::function<bool()> test_func);
    void logTestResult(const std::string& test_name, bool passed, const std::string& details = "");
    
    // 测试助手实例
    std::unique_ptr<RuntimeTestHelper> runtime_helper_;
    std::unique_ptr<HotReloadTester> hotreload_tester_;
    std::unique_ptr<ErrorHandlingTester> error_tester_;
    std::unique_ptr<PerformanceTester> performance_tester_;
    
    // 测试状态
    int total_tests_ = 0;
    int passed_tests_ = 0;
    int failed_tests_ = 0;
    std::chrono::steady_clock::time_point test_start_time_;
    double test_duration_ = 0.0;
    
    // 配置
    bool verbose_output_ = true;
    int test_timeout_ = 300; // 5分钟默认超时
    std::string test_config_;
    
    // 日志
    std::vector<std::string> test_log_;
    std::vector<std::string> failure_details_;
    mutable std::mutex coordinator_mutex_;
};

// ================================
// 全局测试工具函数
// ================================

/**
 * @brief 全局测试工具函数
 */
EXPORT_LUA_FUNCTION()
bool createTestEnvironment();

EXPORT_LUA_FUNCTION()
void cleanupTestEnvironment();

EXPORT_LUA_FUNCTION()
std::string generateTestData(int size);

EXPORT_LUA_FUNCTION()
bool validateTestResult(const std::string& expected, const std::string& actual);

EXPORT_LUA_FUNCTION()
double measureExecutionTime(std::function<void()> test_func);

EXPORT_LUA_FUNCTION()
std::string formatTestReport(const std::vector<std::string>& test_results);

// ================================
// 测试常量和变量
// ================================

EXPORT_LUA_CONSTANT()
static const int MAX_TEST_ITERATIONS = 10000;

EXPORT_LUA_CONSTANT()
static const double TEST_TIMEOUT_SECONDS = 300.0;

EXPORT_LUA_CONSTANT()
static const std::string TEST_VERSION = "2.0.0";

EXPORT_LUA_VARIABLE(access=readwrite)
static bool global_test_verbose = true;

EXPORT_LUA_VARIABLE(access=readwrite)
static int global_test_seed = 12345;

} // namespace runtime_test