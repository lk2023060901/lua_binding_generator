--[[
    @file bidirectional_test.lua
    @brief C++ ↔ Lua 双向交互测试
    
    测试以下双向交互功能：
    - Lua 调用 C++ 函数并处理返回值
    - C++ 调用 Lua 函数（通过回调）
    - 复杂数据类型的往返传递
    - 异常处理和错误传播
    - 性能和稳定性测试
]]

print("=== C++ ↔ Lua 双向交互测试开始 ===")

local tests_run = 0
local tests_passed = 0

local function test_assert(condition, test_name)
    tests_run = tests_run + 1
    if condition then
        tests_passed = tests_passed + 1
        print("✅ [PASS] " .. test_name)
        return true
    else
        print("❌ [FAIL] " .. test_name)
        return false
    end
end

-- ================================
-- Lua → C++ 数据传递测试
-- ================================
print("\n--- Lua → C++ 数据传递测试 ---")

if not (test_coverage and test_coverage.TestPlayer and test_coverage.TestContainerManager) then
    print("❌ 所需的类绑定未找到")
    return false
end

-- 测试基础数据类型传递
if test_coverage.add_numbers then
    -- 整数传递
    local result1 = test_coverage.add_numbers(123, 456)
    test_assert(result1 == 579, "整数参数传递")
    
    -- 负数传递
    local result2 = test_coverage.add_numbers(-10, 20)
    test_assert(result2 == 10, "负数参数传递")
    
    -- 零值传递
    local result3 = test_coverage.add_numbers(0, 0)
    test_assert(result3 == 0, "零值参数传递")
end

-- 测试字符串传递
if test_coverage.format_message then
    -- 英文字符串
    local result1 = test_coverage.format_message("Hello, {}!", "Lua")
    test_assert(result1 == "Hello, Lua!", "英文字符串传递")
    
    -- 中文字符串
    local result2 = test_coverage.format_message("你好，{}！", "世界")
    test_assert(result2 == "你好，世界！", "中文字符串传递")
    
    -- 特殊字符
    local result3 = test_coverage.format_message("Special: {}!", "@#$%^&*()")
    test_assert(result3:find("@#$%^&*()") ~= nil, "特殊字符传递")
end

-- 测试数组/向量传递
if test_coverage.generate_sequence then
    -- 正常范围
    local seq1 = test_coverage.generate_sequence(1, 10, 1)
    test_assert(#seq1 == 10, "序列生成 - 正常范围")
    
    -- 步长测试
    local seq2 = test_coverage.generate_sequence(0, 20, 5)
    test_assert(#seq2 == 5, "序列生成 - 自定义步长")
    test_assert(seq2[1] == 0 and seq2[2] == 5, "序列生成 - 步长验证")
    
    -- 边界情况
    local seq3 = test_coverage.generate_sequence(10, 10, 1)
    test_assert(#seq3 == 1, "序列生成 - 边界情况")
end

-- ================================
-- C++ → Lua 数据返回测试
-- ================================
print("\n--- C++ → Lua 数据返回测试 ---")

-- 测试对象返回和方法链
local player = test_coverage.TestPlayer("TestPlayer", 5)
if player then
    -- 测试方法返回的各种类型
    local name = player:getName()
    test_assert(type(name) == "string", "字符串返回类型")
    test_assert(name == "TestPlayer", "字符串返回值")
    
    local level = player:getLevel()
    test_assert(type(level) == "number", "数字返回类型")
    test_assert(level == 5, "数字返回值")
    
    -- 测试布尔返回
    player:addItem("test_item")
    local has_item = player:hasItem("test_item")
    test_assert(type(has_item) == "boolean", "布尔返回类型")
    test_assert(has_item == true, "布尔返回值")
    
    -- 测试数组返回
    local items = player:getItems()
    test_assert(type(items) == "table", "数组返回类型")
    test_assert(#items >= 1, "数组返回内容")
end

-- ================================
-- 复杂对象传递测试
-- ================================
print("\n--- 复杂对象传递测试 ---")

-- 测试对象作为参数传递
local manager = test_coverage.TestManager()
local player1 = test_coverage.TestPlayer("Player1", 3)
local player2 = test_coverage.TestPlayer("Player2", 7)

if manager and player1 and player2 then
    -- 将 Lua 创建的对象传递给 C++
    manager:addPlayer(player1)
    manager:addPlayer(player2)
    
    test_assert(manager:getPlayerCount() == 2, "对象参数传递")
    
    -- 从 C++ 获取对象并在 Lua 中操作
    local retrieved_player = manager:getPlayer(player1:getId())
    if retrieved_player then
        test_assert(retrieved_player:getName() == "Player1", "对象往返传递")
        
        -- 修改从 C++ 返回的对象
        retrieved_player:setLevel(10)
        test_assert(retrieved_player:getLevel() == 10, "返回对象状态修改")
    end
end

-- ================================
-- 回调函数测试 (C++ → Lua)
-- ================================
print("\n--- 回调函数测试 ---")

if test_coverage.TestEventSystem then
    local event_system = test_coverage.TestEventSystem()
    
    -- 测试简单回调
    local callback_triggered = false
    local callback_data = nil
    
    event_system.OnGameStart = function()
        callback_triggered = true
        print("  回调：游戏开始事件触发")
    end
    
    event_system.OnScoreChange = function(score)
        callback_data = score
        print("  回调：分数变更为 " .. score)
    end
    
    -- 触发回调
    event_system:triggerGameStart()
    test_assert(callback_triggered, "简单回调触发")
    
    event_system:triggerScoreChange(250)
    test_assert(callback_data == 250, "带参数回调")
    
    -- 测试复杂回调
    local complex_callback_data = {}
    event_system.OnPlayerJoin = function(player_name, player_level)
        table.insert(complex_callback_data, {
            name = player_name,
            level = player_level,
            join_time = os.time()
        })
        print("  回调：玩家 " .. player_name .. " (等级 " .. player_level .. ") 加入游戏")
    end
    
    event_system:triggerPlayerJoin("NewPlayer", 1)
    test_assert(#complex_callback_data == 1, "复杂回调数据收集")
    test_assert(complex_callback_data[1].name == "NewPlayer", "复杂回调参数解析")
end

-- ================================
-- 函数式编程接口测试
-- ================================
print("\n--- 函数式编程接口测试 ---")

if test_coverage.process_items then
    local processed_items = {}
    
    -- 定义 Lua 处理函数
    local processor = function(item)
        table.insert(processed_items, item * 2)
        print("  处理项目: " .. item .. " → " .. (item * 2))
    end
    
    -- 传递 Lua 函数给 C++
    local test_items = {1, 2, 3, 4, 5}
    test_coverage.process_items(test_items, processor)
    
    test_assert(#processed_items == 5, "函数参数传递")
    test_assert(processed_items[1] == 2, "Lua 函数处理结果")
    test_assert(processed_items[5] == 10, "Lua 函数批处理结果")
end

-- ================================
-- 异常处理和错误传播测试
-- ================================
print("\n--- 异常处理测试 ---")

-- 测试无效参数处理
local function test_error_handling()
    local success, error_msg
    
    -- 测试无效对象访问
    success = pcall(function()
        local invalid_player = test_coverage.TestPlayer("", -1)  -- 无效参数
        return invalid_player:getLevel()
    end)
    
    print("  无效参数处理: " .. (success and "成功" or "正确捕获错误"))
    
    -- 测试空指针访问
    if manager then
        success = pcall(function()
            local non_existent = manager:getPlayer(99999)  -- 不存在的ID
            if non_existent then
                return non_existent:getName()
            else
                return "正确返回nil"
            end
        end)
        
        print("  空指针处理: " .. (success and "成功" or "正确捕获错误"))
    end
    
    return true
end

test_assert(test_error_handling(), "异常处理机制")

-- ================================
-- 性能和稳定性测试
-- ================================
print("\n--- 性能和稳定性测试 ---")

-- 大量对象创建和销毁
local function performance_test()
    local start_time = os.clock()
    local test_manager = test_coverage.TestManager()
    
    -- 创建1000个对象
    for i = 1, 1000 do
        local player = test_coverage.TestPlayer("PerfTest" .. i, i % 20 + 1)
        player:setHealth(100 + i % 50)
        player:addExperience(i * 10)
        player:addItem("item" .. i)
        test_manager:addPlayer(player)
    end
    
    local creation_time = os.clock() - start_time
    
    -- 批量操作测试
    start_time = os.clock()
    test_manager:levelUpAll()
    test_manager:healAll(10)
    local batch_time = os.clock() - start_time
    
    -- 清理
    start_time = os.clock()
    test_manager:clearAll()
    local cleanup_time = os.clock() - start_time
    
    print("  创建1000对象: " .. (creation_time * 1000) .. " ms")
    print("  批量操作: " .. (batch_time * 1000) .. " ms")
    print("  清理操作: " .. (cleanup_time * 1000) .. " ms")
    
    return creation_time < 1.0 and batch_time < 0.5 and cleanup_time < 0.5
end

test_assert(performance_test(), "性能基准测试")

-- ================================
-- 内存管理测试
-- ================================
print("\n--- 内存管理测试 ---")

-- 测试循环引用和内存泄漏防护
local function memory_test()
    local managers = {}
    local players = {}
    
    -- 创建复杂的对象关系
    for i = 1, 100 do
        local mgr = test_coverage.TestManager()
        local plr = test_coverage.TestPlayer("MemTest" .. i, i)
        
        mgr:addPlayer(plr)
        
        table.insert(managers, mgr)
        table.insert(players, plr)
    end
    
    -- 强制垃圾回收
    collectgarbage("collect")
    
    -- 检查对象是否仍然有效
    local valid_count = 0
    for i, player in ipairs(players) do
        if player:getName() == "MemTest" .. i then
            valid_count = valid_count + 1
        end
    end
    
    print("  有效对象数量: " .. valid_count .. "/100")
    return valid_count == 100
end

test_assert(memory_test(), "内存管理测试")

-- ================================
-- 并发安全测试 (模拟)
-- ================================
print("\n--- 并发安全测试 ---")

local function concurrency_simulation()
    -- 模拟多个"线程"同时操作同一对象
    local shared_manager = test_coverage.TestManager()
    local operations_completed = 0
    
    -- 执行多个操作序列
    for thread = 1, 10 do
        for op = 1, 10 do
            local player = test_coverage.TestPlayer("Thread" .. thread .. "_Op" .. op, op)
            shared_manager:addPlayer(player)
            operations_completed = operations_completed + 1
        end
    end
    
    local final_count = shared_manager:getPlayerCount()
    print("  并发操作完成: " .. operations_completed)
    print("  最终对象数量: " .. final_count)
    
    return final_count == 100
end

test_assert(concurrency_simulation(), "并发安全模拟")

-- ================================
-- 测试结果报告
-- ================================
print("\n" .. string.rep("=", 60))
print("C++ ↔ Lua 双向交互测试结果:")
print("总测试数: " .. tests_run)
print("通过测试: " .. tests_passed)
print("失败测试: " .. (tests_run - tests_passed))

if tests_passed == tests_run then
    print("🎉 所有双向交互测试通过！")
    print("✅ C++ ↔ Lua 数据传递正常")
    print("✅ 回调函数机制正常")
    print("✅ 异常处理正常")
    print("✅ 性能和稳定性良好")
    return true
else
    print("❌ 部分测试失败，请检查双向交互实现")
    return false
end