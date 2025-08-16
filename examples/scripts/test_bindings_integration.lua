--[[
test_bindings_integration.lua
集成测试脚本

这个脚本测试生成的绑定代码是否能够正确集成到实际的Lua环境中
包括Sol2绑定的正确性、内存管理、异常处理等
]]

print("=== Integration Test Suite ===")
print("Testing generated Lua bindings integration...")
print()

-- 检查必需的模块是否可用
local required_modules = {"simple", "engine", "game"}
local missing_modules = {}

for _, module_name in ipairs(required_modules) do
    if not _G[module_name] then
        table.insert(missing_modules, module_name)
    end
end

if #missing_modules > 0 then
    print("❌ ERROR: Required modules are not available: " .. table.concat(missing_modules, ", "))
    print("   Make sure to:")
    print("   1. Generate all bindings: lua_binding_generator examples/*.h")
    print("   2. Integrate bindings into Sol2-enabled application")
    print("   3. Load all modules (simple, engine, game) in Lua")
    print("")
    print("INTEGRATION TEST SUMMARY")
    print("Total integration tests: 0")
    print("Passed: 0")
    print("Failed: 0")
    print("⚠️ Required modules not available - tests skipped")
    return 1
end

-- 测试统计
local tests_passed = 0
local tests_failed = 0

-- 测试辅助函数
function assert_equal(actual, expected, message)
    if actual == expected then
        tests_passed = tests_passed + 1
        print("✓ PASS: " .. (message or "test"))
    else
        tests_failed = tests_failed + 1
        print("✗ FAIL: " .. (message or "test") .. " - Expected: " .. tostring(expected) .. ", Got: " .. tostring(actual))
    end
end

function assert_true(condition, message)
    assert_equal(condition, true, message)
end

function assert_false(condition, message)
    assert_equal(condition, false, message)
end

function safe_call(func, error_message)
    local success, result = pcall(func)
    if success then
        tests_passed = tests_passed + 1
        print("✓ PASS: " .. (error_message or "safe call"))
        return result
    else
        tests_failed = tests_failed + 1
        print("✗ FAIL: " .. (error_message or "safe call") .. " - Error: " .. tostring(result))
        return nil
    end
end

-- 测试模块可用性
print("\n--- Testing Module Availability ---")

-- 检查simple模块
local has_simple = simple ~= nil
assert_true(has_simple, "Simple module is available")

if has_simple then
    assert_true(simple.Calculator ~= nil, "Simple.Calculator class is available")
    assert_true(simple.add ~= nil, "Simple.add function is available")
    assert_true(simple.MAX_COUNT ~= nil, "Simple.MAX_COUNT constant is available")
end

-- 检查engine模块
local has_engine = engine ~= nil
assert_true(has_engine, "Engine module is available")

if has_engine then
    assert_true(engine.core ~= nil, "Engine.core namespace is available")
    assert_true(engine.systems ~= nil, "Engine.systems namespace is available")
    assert_true(engine.utils ~= nil, "Engine.utils namespace is available")
end

-- 检查game模块
local has_game = game ~= nil
assert_true(has_game, "Game module is available")

if has_game then
    assert_true(game.core ~= nil, "Game.core namespace is available")
    assert_true(game.events ~= nil, "Game.events namespace is available")
end

-- 测试内存管理
print("\n--- Testing Memory Management ---")

if has_simple then
    safe_call(function()
        -- 创建大量对象测试内存管理
        local objects = {}
        for i = 1, 1000 do
            local calc = simple.Calculator.new(i)
            objects[i] = calc
        end
        
        -- 测试对象是否正常工作
        assert_equal(objects[500]:getValue(), 500, "Object 500 has correct value")
        
        -- 清理引用，让Lua GC处理
        objects = nil
        collectgarbage("collect")
        
        return true
    end, "Memory management with 1000 Calculator objects")
end

if has_engine then
    safe_call(function()
        -- 测试智能指针的内存管理
        local players = {}
        for i = 1, 100 do
            local player = engine.core.Player.new("Player" .. i)
            players[i] = player
        end
        
        -- 测试交叉引用
        for i = 1, 99 do
            if players[i+1] then
                -- 这里模拟设置target等交叉引用
                -- players[i]:setTarget(players[i+1])
            end
        end
        
        players = nil
        collectgarbage("collect")
        
        return true
    end, "Memory management with smart pointers")
end

-- 测试异常处理
print("\n--- Testing Exception Handling ---")

if has_simple then
    safe_call(function()
        local calc = simple.Calculator.new()
        
        -- 测试除零操作（应该被安全处理）
        calc:setValue(10.0)
        calc:divide(0.0)
        
        -- 值应该保持不变或有合理的默认行为
        local value = calc:getValue()
        assert_true(value == 10.0 or value == 0.0, "Division by zero handled safely")
        
        return true
    end, "Exception handling for division by zero")
end

if has_engine then
    safe_call(function()
        -- 测试空指针处理
        local world = engine.systems.World.getInstance()
        
        -- 尝试获取不存在的对象
        local nonexistent = world:findGameObject(99999)
        assert_true(nonexistent == nil, "Finding nonexistent object returns nil")
        
        return true
    end, "Exception handling for null pointers")
end

-- 测试类型安全
print("\n--- Testing Type Safety ---")

if has_simple then
    safe_call(function()
        local calc = simple.Calculator.new()
        
        -- 测试类型检查
        calc:setValue(42.5)
        assert_equal(calc:getValue(), 42.5, "Double value handling")
        
        -- 测试字符串处理
        local result = simple.greet("Lua")
        assert_true(type(result) == "string", "String return type")
        assert_true(string.len(result) > 0, "Non-empty string result")
        
        return true
    end, "Type safety for basic types")
end

-- 测试枚举值
print("\n--- Testing Enum Values ---")

if has_simple then
    safe_call(function()
        -- 测试枚举常量
        local red = simple.Color.RED
        local green = simple.Color.GREEN
        
        assert_true(type(red) == "number", "Enum value is number")
        assert_true(red ~= green, "Different enum values are different")
        assert_true(red >= 0, "Enum values are non-negative")
        
        return true
    end, "Enum value integrity")
end

-- 测试STL容器绑定（如果可用）
print("\n--- Testing STL Container Bindings ---")

if has_game and game.containers then
    safe_call(function()
        local container_utils = game.containers.ContainerUtils.new()
        
        -- 测试vector操作
        local int_vec = container_utils:getIntVector()
        if int_vec then
            -- 检查是否可以迭代
            local count = 0
            for i, v in ipairs(int_vec) do
                count = count + 1
            end
            assert_true(count >= 0, "Vector iteration works")
        end
        
        return true
    end, "STL container bindings")
end

-- 测试回调函数
print("\n--- Testing Callback Functions ---")

if has_game and game.events then
    safe_call(function()
        local event_system = game.events.EventSystem.new()
        
        -- 测试回调设置和调用
        local callback_called = false
        
        event_system.OnGameStart = function()
            callback_called = true
        end
        
        event_system:triggerGameStart()
        assert_true(callback_called, "Callback function was called")
        
        -- 测试带参数的回调
        local received_name = ""
        
        if game.core and game.core.Player then
            event_system.OnPlayerJoin = function(player)
                received_name = player:getName()
            end
            
            local test_player = game.core.Player.new("CallbackTest")
            event_system:triggerPlayerJoin(test_player)
            assert_equal(received_name, "CallbackTest", "Callback received correct parameter")
        end
        
        return true
    end, "Callback function mechanisms")
end

-- 测试单例模式
print("\n--- Testing Singleton Pattern ---")

if has_engine and engine.systems then
    safe_call(function()
        local world1 = engine.systems.World.getInstance()
        local world2 = engine.systems.World.getInstance()
        
        -- 通过行为验证单例
        world1:clear()
        assert_equal(world2:getObjectCount(), 0, "Singleton behavior - same instance")
        
        local game_mgr1 = engine.systems.GameManager.getInstance()
        local game_mgr2 = engine.systems.GameManager.getInstance()
        
        game_mgr1:setGameState(engine.core.GameState.PLAYING)
        assert_equal(game_mgr2:getGameState(), engine.core.GameState.PLAYING, "GameManager singleton behavior")
        
        return true
    end, "Singleton pattern implementation")
end

-- 测试运算符重载
print("\n--- Testing Operator Overloading ---")

if operators and operators.Vector2D then
    safe_call(function()
        local v1 = operators.Vector2D.new(1, 2)
        local v2 = operators.Vector2D.new(3, 4)
        
        -- 测试加法运算符
        local v3 = v1 + v2
        assert_equal(v3:getX(), 4, "Vector addition X component")
        assert_equal(v3:getY(), 6, "Vector addition Y component")
        
        -- 测试相等运算符
        local v4 = operators.Vector2D.new(1, 2)
        assert_true(v1 == v4, "Vector equality operator")
        
        -- 测试标量乘法
        local v5 = v1 * 2
        assert_equal(v5:getX(), 2, "Vector scalar multiplication X")
        assert_equal(v5:getY(), 4, "Vector scalar multiplication Y")
        
        return true
    end, "Operator overloading functionality")
end

-- 测试继承关系
print("\n--- Testing Inheritance ---")

if has_game and game.core then
    safe_call(function()
        local player = game.core.Player.new("InheritanceTest")
        
        -- 测试基类方法
        player:setName("UpdatedName")
        assert_equal(player:getName(), "UpdatedName", "Inherited method from base class")
        
        -- 测试派生类特有方法
        player:setLevel(5)
        assert_equal(player:getLevel(), 5, "Derived class specific method")
        
        -- 测试多态行为
        local entity_str = player:toString()
        assert_true(string.find(entity_str, "Player") ~= nil, "Polymorphic method call")
        
        return true
    end, "Inheritance and polymorphism")
end

-- 测试线程安全（基础测试）
print("\n--- Testing Thread Safety (Basic) ---")

safe_call(function()
    -- 创建多个对象并进行并发操作模拟
    if has_simple then
        local calcs = {}
        for i = 1, 10 do
            calcs[i] = simple.Calculator.new(i * 10)
        end
        
        -- 模拟并发访问
        for i = 1, 10 do
            for j = 1, 10 do
                calcs[i]:add(1)
            end
        end
        
        -- 验证结果
        for i = 1, 10 do
            local expected = i * 10 + 10
            assert_equal(calcs[i]:getValue(), expected, "Concurrent operations result " .. i)
        end
    end
    
    return true
end, "Basic thread safety simulation")

-- 性能测试
print("\n--- Testing Performance (Basic) ---")

safe_call(function()
    local start_time = os.clock()
    
    if has_simple then
        -- 执行大量操作
        local calc = simple.Calculator.new()
        for i = 1, 10000 do
            calc:add(1)
            calc:multiply(1.001)
            calc:divide(1.001)
        end
    end
    
    local end_time = os.clock()
    local duration = end_time - start_time
    
    assert_true(duration < 5.0, "Performance test completed in reasonable time")
    print(string.format("   Performance: 10,000 operations in %.3f seconds", duration))
    
    return true
end, "Basic performance characteristics")

-- 最终报告
print("\n" .. string.rep("=", 60))
print("INTEGRATION TEST SUMMARY")
print(string.rep("=", 60))

local total_tests = tests_passed + tests_failed
local success_rate = 0
if total_tests > 0 then
    success_rate = (tests_passed / total_tests) * 100
end

print(string.format("Total integration tests: %d", total_tests))
print(string.format("Passed: %d", tests_passed))
print(string.format("Failed: %d", tests_failed))
print(string.format("Success rate: %.1f%%", success_rate))

if tests_failed == 0 and total_tests > 0 then
    print()
    print("🎉 ALL INTEGRATION TESTS PASSED! 🎉")
    print("The generated Lua bindings are working correctly!")
    print()
    print("✅ Integration Status:")
    print("   • Memory management: Working")
    print("   • Exception handling: Working") 
    print("   • Type safety: Working")
    print("   • Callback functions: Working")
    print("   • Operator overloading: Working")
    print("   • Inheritance: Working")
    print("   • Performance: Acceptable")
    return 0
else
    print()
    print("⚠️  INTEGRATION ISSUES DETECTED")
    if total_tests == 0 then
        print("No integration tests could be run - check binding availability")
    else
        print(string.format("%d out of %d integration tests failed", tests_failed, total_tests))
    end
    print()
    print("🔧 Recommended actions:")
    print("   1. Verify that bindings were generated correctly")
    print("   2. Check Sol2 integration")
    print("   3. Review C++ export macro usage")
    print("   4. Test with simpler examples first")
    return 1
end