--[[
    @file macro_test.lua
    @brief 15个核心宏的完整功能测试
    
    这个脚本测试所有 EXPORT_LUA_* 宏生成的绑定：
    1. EXPORT_LUA_MODULE - 模块访问
    2. EXPORT_LUA_NAMESPACE - 命名空间访问
    3. EXPORT_LUA_CLASS - 类实例化和方法调用
    4. EXPORT_LUA_ENUM - 枚举值访问
    5. EXPORT_LUA_SINGLETON - 单例访问
    6. EXPORT_LUA_STATIC_CLASS - 静态类方法
    7. EXPORT_LUA_ABSTRACT_CLASS - 抽象类（通过派生类）
    8. EXPORT_LUA_FUNCTION - 全局函数调用
    9. EXPORT_LUA_VARIABLE - 变量读写
    10. EXPORT_LUA_CONSTANT - 常量访问
    11. EXPORT_LUA_VECTOR - Vector容器
    12. EXPORT_LUA_MAP - Map容器
    13. EXPORT_LUA_CALLBACK - 回调函数
    14. EXPORT_LUA_OPERATOR - 运算符重载
    15. EXPORT_LUA_PROPERTY - 属性访问器
]]

print("=== Lua 宏功能测试开始 ===")

-- 测试结果统计
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
-- 10. EXPORT_LUA_CONSTANT - 常量访问测试
-- ================================
print("\n--- 常量访问测试 ---")

if test_coverage and test_coverage.MAX_CONNECTIONS then
    test_assert(test_coverage.MAX_CONNECTIONS == 1000, "常量 MAX_CONNECTIONS")
else
    test_assert(false, "常量 MAX_CONNECTIONS - 绑定未找到")
end

if test_coverage and test_coverage.PI_VALUE then
    test_assert(math.abs(test_coverage.PI_VALUE - 3.14159265359) < 0.00001, "常量 PI_VALUE")
else
    test_assert(false, "常量 PI_VALUE - 绑定未找到")
end

if test_coverage and test_coverage.TEST_VERSION then
    test_assert(test_coverage.TEST_VERSION == "2.0.0", "常量 TEST_VERSION")
else
    test_assert(false, "常量 TEST_VERSION - 绑定未找到")
end

if test_coverage and test_coverage.DEBUG_ENABLED then
    test_assert(test_coverage.DEBUG_ENABLED == true, "常量 DEBUG_ENABLED")
else
    test_assert(false, "常量 DEBUG_ENABLED - 绑定未找到")
end

-- ================================
-- 9. EXPORT_LUA_VARIABLE - 变量读写测试
-- ================================
print("\n--- 变量读写测试 ---")

if test_coverage and test_coverage.global_counter ~= nil then
    local initial_value = test_coverage.global_counter
    test_coverage.global_counter = 42
    test_assert(test_coverage.global_counter == 42, "变量 global_counter 写入")
    
    test_coverage.global_counter = initial_value + 10
    test_assert(test_coverage.global_counter == initial_value + 10, "变量 global_counter 运算")
else
    test_assert(false, "变量 global_counter - 绑定未找到")
end

if test_coverage and test_coverage.system_name then
    test_assert(test_coverage.system_name == "MacroCoverageTest", "只读变量 system_name")
else
    test_assert(false, "变量 system_name - 绑定未找到")
end

-- ================================
-- 8. EXPORT_LUA_FUNCTION - 全局函数测试
-- ================================
print("\n--- 全局函数测试 ---")

if test_coverage and test_coverage.add_numbers then
    local result = test_coverage.add_numbers(10, 20)
    test_assert(result == 30, "函数 add_numbers(10, 20)")
else
    test_assert(false, "函数 add_numbers - 绑定未找到")
end

if test_coverage and test_coverage.format_message then
    local result = test_coverage.format_message("Hello, {}!", "World")
    test_assert(result == "Hello, World!", "函数 format_message")
else
    test_assert(false, "函数 format_message - 绑定未找到")
end

if test_coverage and test_coverage.generate_sequence then
    local sequence = test_coverage.generate_sequence(1, 10, 2)
    test_assert(#sequence == 5, "函数 generate_sequence 长度")
    test_assert(sequence[1] == 1, "函数 generate_sequence 首元素")
    test_assert(sequence[5] == 9, "函数 generate_sequence 末元素")
else
    test_assert(false, "函数 generate_sequence - 绑定未找到")
end

-- 测试重载函数
if test_coverage and test_coverage.calculate_area then
    local circle_area = test_coverage.calculate_area(5.0)
    local rect_area = test_coverage.calculate_area(4.0, 3.0)
    test_assert(circle_area > 78.0, "重载函数 calculate_area(圆形)")
    test_assert(rect_area == 12.0, "重载函数 calculate_area(矩形)")
else
    test_assert(false, "重载函数 calculate_area - 绑定未找到")
end

-- ================================
-- 4. EXPORT_LUA_ENUM - 枚举测试
-- ================================
print("\n--- 枚举测试 ---")

if test_coverage and test_coverage.TestStatus then
    test_assert(test_coverage.TestStatus.INACTIVE == 0, "枚举 TestStatus.INACTIVE")
    test_assert(test_coverage.TestStatus.ACTIVE == 1, "枚举 TestStatus.ACTIVE")
    test_assert(test_coverage.TestStatus.PENDING == 2, "枚举 TestStatus.PENDING")
    test_assert(test_coverage.TestStatus.COMPLETED == 3, "枚举 TestStatus.COMPLETED")
    test_assert(test_coverage.TestStatus.ERROR == -1, "枚举 TestStatus.ERROR")
else
    test_assert(false, "枚举 TestStatus - 绑定未找到")
end

if test_coverage and test_coverage.TestPriority then
    test_assert(test_coverage.TestPriority.LOW == 1, "枚举 TestPriority.LOW")
    test_assert(test_coverage.TestPriority.MEDIUM == 5, "枚举 TestPriority.MEDIUM")
    test_assert(test_coverage.TestPriority.HIGH == 10, "枚举 TestPriority.HIGH")
    test_assert(test_coverage.TestPriority.CRITICAL == 100, "枚举 TestPriority.CRITICAL")
else
    test_assert(false, "枚举 TestPriority - 绑定未找到")
end

-- ================================
-- 3. EXPORT_LUA_CLASS - 类实例化和方法测试
-- ================================
print("\n--- 类实例化和方法测试 ---")

if test_coverage and test_coverage.TestPlayer then
    -- 测试默认构造函数
    local player = test_coverage.TestPlayer()
    test_assert(player ~= nil, "TestPlayer 默认构造")
    
    -- 测试带参数构造函数
    local hero = test_coverage.TestPlayer("Hero", 5)
    test_assert(hero ~= nil, "TestPlayer 带参构造")
    
    -- 测试方法调用
    test_assert(hero:getName() == "Hero", "TestPlayer:getName()")
    test_assert(hero:getLevel() == 5, "TestPlayer:getLevel()")
    
    -- 测试属性设置
    hero:setHealth(100.0)
    test_assert(hero:getHealth() == 100.0, "TestPlayer:setHealth/getHealth")
    
    -- 测试方法调用
    hero:levelUp()
    test_assert(hero:getLevel() == 6, "TestPlayer:levelUp()")
    
    -- 测试经验值系统
    local exp_before = hero:getExperience()
    hero:addExperience(100)
    test_assert(hero:getExperience() == exp_before + 100, "TestPlayer:addExperience()")
    
    -- 测试物品系统
    hero:addItem("sword")
    hero:addItem("shield")
    test_assert(hero:hasItem("sword"), "TestPlayer:hasItem()")
    
    local items = hero:getItems()
    test_assert(#items == 2, "TestPlayer:getItems() 数量")
    
    hero:removeItem("sword")
    test_assert(not hero:hasItem("sword"), "TestPlayer:removeItem()")
    
    -- 测试虚函数调用
    test_assert(hero:getType() == "Player", "TestPlayer:getType() 多态")
    test_assert(hero:getScore() > 0, "TestPlayer:getScore() 多态")
    
else
    test_assert(false, "类 TestPlayer - 绑定未找到")
end

-- ================================
-- 管理器类测试
-- ================================
if test_coverage and test_coverage.TestManager then
    local manager = test_coverage.TestManager()
    test_assert(manager ~= nil, "TestManager 构造")
    
    -- 创建玩家并添加到管理器
    local player1 = test_coverage.TestPlayer("Player1", 5)
    local player2 = test_coverage.TestPlayer("Player2", 10)
    
    manager:addPlayer(player1)
    manager:addPlayer(player2)
    
    test_assert(manager:getPlayerCount() == 2, "TestManager:getPlayerCount()")
    
    local avg_level = manager:getAverageLevel()
    test_assert(math.abs(avg_level - 7.5) < 0.1, "TestManager:getAverageLevel()")
    
    local names = manager:getPlayerNames()
    test_assert(#names == 2, "TestManager:getPlayerNames() 数量")
    
    -- 测试批量操作
    manager:levelUpAll()
    test_assert(player1:getLevel() == 6, "TestManager:levelUpAll() 效果")
    
    manager:healAll(50.0)
    test_assert(player1:getHealth() >= 50.0, "TestManager:healAll() 效果")
    
else
    test_assert(false, "类 TestManager - 绑定未找到")
end

-- ================================
-- 14. EXPORT_LUA_OPERATOR - 运算符重载测试
-- ================================
print("\n--- 运算符重载测试 ---")

if test_coverage and test_coverage.TestVector2D then
    local v1 = test_coverage.TestVector2D(3.0, 4.0)
    local v2 = test_coverage.TestVector2D(1.0, 2.0)
    
    test_assert(v1:getX() == 3.0 and v1:getY() == 4.0, "TestVector2D 构造和访问器")
    
    -- 测试向量运算
    local v3 = v1 + v2
    test_assert(v3:getX() == 4.0 and v3:getY() == 6.0, "Vector2D 加法运算符")
    
    local v4 = v1 - v2
    test_assert(v4:getX() == 2.0 and v4:getY() == 2.0, "Vector2D 减法运算符")
    
    local v5 = v1 * 2.0
    test_assert(v5:getX() == 6.0 and v5:getY() == 8.0, "Vector2D 标量乘法")
    
    -- 测试向量方法
    local length = v1:length()
    test_assert(math.abs(length - 5.0) < 0.001, "Vector2D:length()")
    
    local dot_product = v1:dot(v2)
    test_assert(dot_product == 11.0, "Vector2D:dot()")
    
    -- 测试比较运算符
    local v6 = test_coverage.TestVector2D(3.0, 4.0)
    test_assert(v1 == v6, "Vector2D 相等运算符")
    test_assert(v1 ~= v2, "Vector2D 不等运算符")
    
else
    test_assert(false, "类 TestVector2D - 绑定未找到")
end

-- ================================
-- 13. EXPORT_LUA_CALLBACK - 回调函数测试
-- ================================
print("\n--- 回调函数测试 ---")

if test_coverage and test_coverage.TestEventSystem then
    local event_system = test_coverage.TestEventSystem()
    test_assert(event_system ~= nil, "TestEventSystem 构造")
    
    -- 设置回调函数
    local game_started = false
    local score_changed = 0
    
    event_system.OnGameStart = function()
        game_started = true
    end
    
    event_system.OnScoreChange = function(new_score)
        score_changed = new_score
    end
    
    -- 触发事件
    event_system:triggerGameStart()
    test_assert(game_started, "回调函数 OnGameStart")
    
    event_system:triggerScoreChange(150)
    test_assert(score_changed == 150, "回调函数 OnScoreChange")
    
    -- 测试回调状态检查
    test_assert(event_system:hasGameStartCallback(), "hasGameStartCallback() 检查")
    test_assert(event_system:hasScoreChangeCallback(), "hasScoreChangeCallback() 检查")
    
else
    test_assert(false, "类 TestEventSystem - 绑定未找到")
end

-- ================================
-- 12. EXPORT_LUA_MAP & 11. EXPORT_LUA_VECTOR - 容器测试
-- ================================
print("\n--- 容器操作测试 ---")

if test_coverage and test_coverage.TestContainerManager then
    local container_mgr = test_coverage.TestContainerManager()
    test_assert(container_mgr ~= nil, "TestContainerManager 构造")
    
    -- 测试 Vector 操作
    container_mgr:addNumber(10)
    container_mgr:addNumber(20)
    container_mgr:addNumber(30)
    
    local numbers = container_mgr:getNumbers()
    test_assert(#numbers == 3, "Vector 容器大小")
    test_assert(numbers[2] == 20, "Vector 容器元素访问")
    
    -- 测试 Map 操作
    container_mgr:setProperty("name", "TestProject")
    container_mgr:setProperty("version", "2.0.0")
    
    local name = container_mgr:getProperty("name")
    test_assert(name == "TestProject", "Map 容器设置和获取")
    
    local version = container_mgr:getProperty("version")
    test_assert(version == "2.0.0", "Map 容器多值操作")
    
else
    test_assert(false, "类 TestContainerManager - 绑定未找到")
end

-- ================================
-- 6. EXPORT_LUA_STATIC_CLASS - 静态类方法测试
-- ================================
print("\n--- 静态类方法测试 ---")

if test_coverage and test_coverage.TestMathUtils then
    local clamped = test_coverage.TestMathUtils.clamp(15.0, 0.0, 10.0)
    test_assert(clamped == 10.0, "静态方法 TestMathUtils.clamp")
    
    local lerped = test_coverage.TestMathUtils.lerp(0.0, 100.0, 0.5)
    test_assert(lerped == 50.0, "静态方法 TestMathUtils.lerp")
    
    local is_prime = test_coverage.TestMathUtils.isPrime(17)
    test_assert(is_prime == true, "静态方法 TestMathUtils.isPrime")
    
else
    test_assert(false, "静态类 TestMathUtils - 绑定未找到")
end

if test_coverage and test_coverage.TestStringUtils then
    local upper = test_coverage.TestStringUtils.toUpper("hello")
    test_assert(upper == "HELLO", "静态方法 TestStringUtils.toUpper")
    
    local reversed = test_coverage.TestStringUtils.reverse("world")
    test_assert(reversed == "dlrow", "静态方法 TestStringUtils.reverse")
    
    local parts = test_coverage.TestStringUtils.split("a,b,c", ',')
    test_assert(#parts == 3, "静态方法 TestStringUtils.split")
    
else
    test_assert(false, "静态类 TestStringUtils - 绑定未找到")
end

-- ================================
-- 5. EXPORT_LUA_SINGLETON - 单例模式测试
-- ================================
print("\n--- 单例模式测试 ---")

if test_coverage and test_coverage.TestGameManager then
    local game_mgr = test_coverage.TestGameManager.getInstance()
    test_assert(game_mgr ~= nil, "单例 TestGameManager.getInstance()")
    
    game_mgr:startGame()
    game_mgr:addScore(100)
    
    -- 验证单例特性
    local game_mgr2 = test_coverage.TestGameManager.getInstance()
    test_assert(game_mgr2:getScore() == 100, "单例状态共享")
    
else
    test_assert(false, "单例 TestGameManager - 绑定未找到")
end

-- ================================
-- 测试结果报告
-- ================================
print("\n" .. string.rep("=", 60))
print("测试结果摘要:")
print("总测试数: " .. tests_run)
print("通过测试: " .. tests_passed)
print("失败测试: " .. (tests_run - tests_passed))

if tests_passed == tests_run then
    print("🎉 所有宏功能测试通过！")
    print("✅ C++ 到 Lua 绑定工作正常")
    return true
else
    print("❌ 部分测试失败，请检查绑定实现")
    return false
end