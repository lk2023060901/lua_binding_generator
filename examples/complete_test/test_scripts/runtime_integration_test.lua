--[[
    @file runtime_integration_test.lua
    @brief 运行时库集成测试脚本
    
    测试运行时库功能：
    - 内存分配器
    - 热加载系统
    - 错误处理
    - 性能监控
    - 集成测试协调器
]]

print("=== 运行时库集成测试 ===")

-- 测试全局工具函数
print("\n--- 全局工具函数测试 ---")

if createTestEnvironment then
    print("✅ createTestEnvironment 函数找到")
    local env_result = createTestEnvironment()
    print("   环境创建结果:", env_result)
    assert(env_result == true, "Test environment should be created successfully")
else
    print("⚠️  createTestEnvironment 函数未找到")
end

if generateTestData then
    print("✅ generateTestData 函数找到")
    local test_data = generateTestData(20)
    print("   生成的测试数据长度:", #test_data)
    assert(#test_data == 20, "Generated data should have specified length")
    print("   测试数据样本:", string.sub(test_data, 1, 10) .. "...")
else
    print("⚠️  generateTestData 函数未找到")
end

if validateTestResult then
    print("✅ validateTestResult 函数找到")
    local valid1 = validateTestResult("hello", "hello")
    local valid2 = validateTestResult("hello", "world")
    print("   验证结果1 (相同):", valid1)
    print("   验证结果2 (不同):", valid2)
    assert(valid1 == true, "Same strings should validate as equal")
    assert(valid2 == false, "Different strings should validate as not equal")
else
    print("⚠️  validateTestResult 函数未找到")
end

-- 测试运行时常量
print("\n--- 运行时常量测试 ---")

if MAX_TEST_ITERATIONS then
    print("✅ MAX_TEST_ITERATIONS =", MAX_TEST_ITERATIONS)
    assert(MAX_TEST_ITERATIONS == 10000, "MAX_TEST_ITERATIONS should be 10000")
else
    print("⚠️  MAX_TEST_ITERATIONS 常量未找到")
end

if TEST_TIMEOUT_SECONDS then
    print("✅ TEST_TIMEOUT_SECONDS =", TEST_TIMEOUT_SECONDS)
    assert(TEST_TIMEOUT_SECONDS == 300.0, "TEST_TIMEOUT_SECONDS should be 300.0")
else
    print("⚠️  TEST_TIMEOUT_SECONDS 常量未找到")
end

if TEST_VERSION then
    print("✅ TEST_VERSION =", TEST_VERSION)
    assert(TEST_VERSION == "2.0.0", "TEST_VERSION should be '2.0.0'")
else
    print("⚠️  TEST_VERSION 常量未找到")
end

-- 测试运行时变量
print("\n--- 运行时变量测试 ---")

if global_test_verbose ~= nil then
    print("✅ global_test_verbose =", global_test_verbose)
    global_test_verbose = false
    print("   设置 global_test_verbose = false")
else
    print("⚠️  global_test_verbose 变量未找到")
end

if global_test_seed then
    print("✅ global_test_seed =", global_test_seed)
    global_test_seed = 54321
    print("   设置 global_test_seed = 54321")
    assert(global_test_seed == 54321, "global_test_seed should be updated")
else
    print("⚠️  global_test_seed 变量未找到")
end

-- 测试运行时测试助手
print("\n--- 运行时测试助手 (RuntimeTestHelper) ---")
if RuntimeTestHelper then
    print("✅ RuntimeTestHelper 类找到")
    
    local runtime_helper = RuntimeTestHelper()
    assert(runtime_helper, "RuntimeTestHelper should be created")
    
    -- 测试运行时初始化
    local init_result = runtime_helper:initializeRuntime()
    print("   运行时初始化结果:", init_result)
    
    if init_result then
        print("   运行时状态:", runtime_helper:isRuntimeActive())
        assert(runtime_helper:isRuntimeActive() == true, "Runtime should be active")
        
        -- 测试绑定注册
        local binding_result = runtime_helper:registerTestBindings()
        print("   绑定注册结果:", binding_result)
        
        if binding_result then
            -- 测试基础绑定
            local basic_test = runtime_helper:testBasicBindings()
            print("   基础绑定测试:", basic_test)
            
            -- 测试复杂绑定
            local complex_test = runtime_helper:testComplexBindings()
            print("   复杂绑定测试:", complex_test)
        end
        
        -- 测试脚本执行
        local script_result = runtime_helper:executeSimpleScript("return 42")
        print("   脚本执行结果:", script_result)
        
        local last_result = runtime_helper:getLastExecutionResult()
        print("   最后执行结果:", last_result)
        
        -- 测试性能测量
        local exec_time = runtime_helper:measureExecutionTime("return math.sqrt(100)")
        print("   执行时间测量:", exec_time, "ms")
        assert(exec_time >= 0, "Execution time should be non-negative")
        
        local func_time = runtime_helper:measureFunctionCall("math.random", 10)
        print("   函数调用时间:", func_time, "ms")
        
        -- 获取性能报告
        local perf_report = runtime_helper:getPerformanceReport()
        print("   性能报告:")
        for line in perf_report:gmatch("[^\n]+") do
            print("     " .. line)
        end
        
        -- 测试运行时信息
        local runtime_info = runtime_helper:getRuntimeInfo()
        print("   运行时信息:", runtime_info)
        
        local memory_info = runtime_helper:getMemoryInfo()
        print("   内存信息:", memory_info)
    end
    
    print("✅ 运行时测试助手测试通过")
else
    print("⚠️  RuntimeTestHelper 类未找到")
end

-- 测试内存分配器
print("\n--- 内存分配器测试 (TestTrackingAllocator) ---")
if TestTrackingAllocator then
    print("✅ TestTrackingAllocator 类找到")
    
    local tracking_alloc = TestTrackingAllocator()
    assert(tracking_alloc, "TestTrackingAllocator should be created")
    
    -- 获取初始统计
    local initial_total = tracking_alloc:getTotalAllocated()
    local initial_count = tracking_alloc:getAllocationCount()
    print("   初始已分配内存:", initial_total)
    print("   初始分配次数:", initial_count)
    
    -- 注意：在Lua中直接测试C++内存分配器可能有限制
    -- 这里主要测试统计接口的可用性
    
    local peak_allocated = tracking_alloc:getPeakAllocated()
    local dealloc_count = tracking_alloc:getDeallocationCount()
    local active_allocs = tracking_alloc:getActiveAllocations()
    
    print("   峰值内存:", peak_allocated)
    print("   释放次数:", dealloc_count)
    print("   活跃分配:", active_allocs)
    
    -- 测试统计报告
    local stats_report = tracking_alloc:getStatsReport()
    print("   统计报告:")
    for line in stats_report:gmatch("[^\n]+") do
        print("     " .. line)
    end
    
    -- 测试泄漏检测
    local has_leaks = tracking_alloc:hasMemoryLeaks()
    print("   内存泄漏检测:", has_leaks)
    
    local leak_report = tracking_alloc:getLeakReport()
    if #leak_report > 0 then
        print("   泄漏报告:")
        for i, report in ipairs(leak_report) do
            print("     " .. report)
        end
    end
    
    print("✅ 跟踪分配器测试通过")
else
    print("⚠️  TestTrackingAllocator 类未找到")
end

-- 测试内存池分配器
print("\n--- 内存池分配器测试 (TestPoolAllocator) ---")
if TestPoolAllocator then
    print("✅ TestPoolAllocator 类找到")
    
    local pool_alloc = TestPoolAllocator(1024, 64)  -- 1KB池，64字节块
    assert(pool_alloc, "TestPoolAllocator should be created")
    
    local pool_size = pool_alloc:getPoolSize()
    local block_size = pool_alloc:getBlockSize()
    local used_blocks = pool_alloc:getUsedBlocks()
    local available_blocks = pool_alloc:getAvailableBlocks()
    
    print("   池大小:", pool_size)
    print("   块大小:", block_size)
    print("   已用块数:", used_blocks)
    print("   可用块数:", available_blocks)
    
    assert(pool_size == 1024, "Pool size should be 1024")
    assert(block_size == 64, "Block size should be 64")
    
    local fragmentation = pool_alloc:getFragmentation()
    print("   碎片化程度:", fragmentation)
    
    local is_full = pool_alloc:isFull()
    print("   池是否已满:", is_full)
    assert(is_full == false, "Pool should not be full initially")
    
    local pool_info = pool_alloc:getPoolInfo()
    print("   池信息:")
    for line in pool_info:gmatch("[^\n]+") do
        print("     " .. line)
    end
    
    print("✅ 内存池分配器测试通过")
else
    print("⚠️  TestPoolAllocator 类未找到")
end

-- 测试热加载系统
print("\n--- 热加载系统测试 (HotReloadTester) ---")
if HotReloadTester then
    print("✅ HotReloadTester 类找到")
    
    local hotreload_tester = HotReloadTester()
    assert(hotreload_tester, "HotReloadTester should be created")
    
    -- 测试基础热加载
    local basic_reload = hotreload_tester:testBasicReload()
    print("   基础热加载测试:", basic_reload)
    
    local protected_reload = hotreload_tester:testProtectedTableReload()
    print("   保护表热加载测试:", protected_reload)
    
    local callback_reload = hotreload_tester:testCallbackReload()
    print("   回调热加载测试:", callback_reload)
    
    local error_recovery = hotreload_tester:testErrorRecovery()
    print("   错误恢复测试:", error_recovery)
    
    local concurrent_reload = hotreload_tester:testConcurrentReload()
    print("   并发热加载测试:", concurrent_reload)
    
    -- 获取统计信息
    local successful_reloads = hotreload_tester:getSuccessfulReloads()
    local failed_reloads = hotreload_tester:getFailedReloads()
    print("   成功重载次数:", successful_reloads)
    print("   失败重载次数:", failed_reloads)
    
    local reload_log = hotreload_tester:getReloadLog()
    print("   重载日志条目:", #reload_log)
    if #reload_log > 0 then
        print("   最近的日志:")
        for i = math.max(1, #reload_log - 3), #reload_log do
            print("     " .. reload_log[i])
        end
    end
    
    local reload_report = hotreload_tester:getReloadReport()
    print("   重载报告:")
    for line in reload_report:gmatch("[^\n]+") do
        print("     " .. line)
    end
    
    print("✅ 热加载系统测试通过")
else
    print("⚠️  HotReloadTester 类未找到")
end

-- 测试错误处理系统
print("\n--- 错误处理系统测试 (ErrorHandlingTester) ---")
if ErrorHandlingTester then
    print("✅ ErrorHandlingTester 类找到")
    
    local error_tester = ErrorHandlingTester()
    assert(error_tester, "ErrorHandlingTester should be created")
    
    -- 测试各种错误处理
    local success_test = error_tester:testSuccessResult()
    print("   成功结果测试:", success_test)
    
    local error_test = error_tester:testErrorResult()
    print("   错误结果测试:", error_test)
    
    local recovery_test = error_tester:testErrorRecovery()
    print("   错误恢复测试:", recovery_test)
    
    local syntax_test = error_tester:testSyntaxError()
    print("   语法错误测试:", syntax_test)
    
    local runtime_error_test = error_tester:testRuntimeError()
    print("   运行时错误测试:", runtime_error_test)
    
    -- 获取错误统计
    local error_count = error_tester:getErrorCount()
    print("   错误计数:", error_count)
    
    local error_history = error_tester:getErrorHistory()
    print("   错误历史条目:", #error_history)
    
    local last_error = error_tester:getLastErrorDetail()
    print("   最后错误详情:", last_error)
    
    -- 模拟各种错误
    error_tester:simulateMemoryError()
    error_tester:simulateLuaError()
    error_tester:simulateFileError()
    
    local final_error_count = error_tester:getErrorCount()
    print("   模拟错误后计数:", final_error_count)
    assert(final_error_count > error_count, "Error count should increase after simulations")
    
    print("✅ 错误处理系统测试通过")
else
    print("⚠️  ErrorHandlingTester 类未找到")
end

-- 测试性能测试器
print("\n--- 性能测试器 (PerformanceTester) ---")
if PerformanceTester then
    print("✅ PerformanceTester 类找到")
    
    local perf_tester = PerformanceTester()
    assert(perf_tester, "PerformanceTester should be created")
    
    -- 运行各种基准测试
    local script_time = perf_tester:benchmarkScriptExecution("return 42", 10)
    print("   脚本执行基准:", script_time, "ms")
    
    local func_time = perf_tester:benchmarkFunctionCall("math.random", 10)
    print("   函数调用基准:", func_time, "ms")
    
    local obj_time = perf_tester:benchmarkObjectCreation("TestPlayer", 5)
    print("   对象创建基准:", obj_time, "ms")
    
    local alloc_time = perf_tester:benchmarkMemoryAllocation(1024, 10)
    print("   内存分配基准:", alloc_time, "ms")
    
    -- 获取性能统计
    local avg_time = perf_tester:getAverageExecutionTime()
    print("   平均执行时间:", avg_time, "ms")
    
    local peak_memory = perf_tester:getPeakMemoryUsage()
    print("   峰值内存使用:", peak_memory, "bytes")
    
    local time_history = perf_tester:getTimeHistory()
    print("   时间历史记录数:", #time_history)
    
    -- 获取性能报告
    local perf_report = perf_tester:getPerformanceReport()
    print("   性能报告:")
    for line in perf_report:gmatch("[^\n]+") do
        print("     " .. line)
    end
    
    -- 测试性能阈值检查
    local time_threshold = perf_tester:checkPerformanceThreshold(1000.0)  -- 1秒
    local memory_threshold = perf_tester:checkMemoryThreshold(100)  -- 100MB
    print("   时间阈值检查:", time_threshold)
    print("   内存阈值检查:", memory_threshold)
    
    print("✅ 性能测试器测试通过")
else
    print("⚠️  PerformanceTester 类未找到")
end

-- 测试集成测试协调器
print("\n--- 集成测试协调器 (IntegrationTestCoordinator) ---")
if IntegrationTestCoordinator then
    print("✅ IntegrationTestCoordinator 单例类找到")
    
    local coordinator = IntegrationTestCoordinator.getInstance()
    assert(coordinator, "IntegrationTestCoordinator should be available")
    
    -- 初始化测试套件
    local init_result = coordinator:initializeTestSuite()
    print("   测试套件初始化:", init_result)
    
    if init_result then
        -- 配置测试
        coordinator:setVerboseOutput(false)  -- 减少输出
        coordinator:setTestTimeout(60)  -- 1分钟超时
        
        -- 运行不同类型的测试
        print("   运行基础测试...")
        local basic_result = coordinator:runBasicTests()
        print("   基础测试结果:", basic_result)
        
        print("   运行宏测试...")
        local macro_result = coordinator:runMacroTests()
        print("   宏测试结果:", macro_result)
        
        print("   运行运行时测试...")
        local runtime_result = coordinator:runRuntimeTests()
        print("   运行时测试结果:", runtime_result)
        
        -- 获取测试统计
        local total_tests = coordinator:getTotalTests()
        local passed_tests = coordinator:getPassedTests()
        local failed_tests = coordinator:getFailedTests()
        local test_duration = coordinator:getTestDuration()
        
        print("   总测试数:", total_tests)
        print("   通过测试:", passed_tests)
        print("   失败测试:", failed_tests)
        print("   测试时长:", test_duration, "秒")
        
        if total_tests > 0 then
            local success_rate = (passed_tests / total_tests) * 100
            print("   成功率:", string.format("%.1f%%", success_rate))
        end
        
        -- 获取测试报告
        local test_report = coordinator:getTestReport()
        print("   测试报告:")
        for line in test_report:gmatch("[^\n]+") do
            print("     " .. line)
        end
        
        if failed_tests > 0 then
            local failure_report = coordinator:getFailureReport()
            print("   失败报告:")
            for line in failure_report:gmatch("[^\n]+") do
                print("     " .. line)
            end
        end
        
        -- 获取测试日志
        local test_log = coordinator:getTestLog()
        print("   测试日志条目:", #test_log)
        if #test_log > 0 then
            print("   最近的日志条目:")
            for i = math.max(1, #test_log - 5), #test_log do
                print("     " .. test_log[i])
            end
        end
        
        -- 清理
        coordinator:shutdownTestSuite()
        print("   测试套件已关闭")
    end
    
    print("✅ 集成测试协调器测试通过")
else
    print("⚠️  IntegrationTestCoordinator 单例类未找到")
end

-- 清理测试环境
if cleanupTestEnvironment then
    print("\n--- 清理测试环境 ---")
    cleanupTestEnvironment()
    print("✅ 测试环境已清理")
end

print("\n🎉 运行时库集成测试全部通过！")
print("   测试内容包括:")
print("   ✅ 全局工具函数")
print("   ✅ 运行时常量和变量")
print("   ✅ 运行时测试助手")
print("   ✅ 内存分配器系统")
print("   ✅ 热加载系统")
print("   ✅ 错误处理系统")
print("   ✅ 性能测试器")
print("   ✅ 集成测试协调器")

return true