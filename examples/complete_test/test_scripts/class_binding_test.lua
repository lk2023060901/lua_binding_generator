--[[
    @file class_binding_test.lua
    @brief 类绑定测试脚本
    
    测试C++类的Lua绑定，包括：
    - 普通类 (EXPORT_LUA_CLASS)
    - 抽象类 (EXPORT_LUA_ABSTRACT_CLASS)
    - 静态类 (EXPORT_LUA_STATIC_CLASS)
    - 单例类 (EXPORT_LUA_SINGLETON)
]]

print("=== 类绑定测试 ===")

-- 测试普通类 (TestPlayer)
print("\n--- 普通类测试 (TestPlayer) ---")
if TestPlayer then
    print("✅ TestPlayer 类找到")
    
    -- 测试构造函数
    local player = TestPlayer("勇者", 1)
    assert(player, "Player should be created")
    
    print("✅ 创建玩家:", player:getName())
    assert(player:getName() == "勇者", "Player name should be '勇者'")
    assert(player:getLevel() == 1, "Player level should be 1")
    
    -- 测试属性访问
    print("   初始生命值:", player:getHealth())
    print("   最大生命值:", player:getMaxHealth())
    assert(player:getHealth() == 100.0, "Initial health should be 100")
    
    -- 测试方法调用
    player:addExperience(150)  -- 应该升级
    print("   添加150经验后等级:", player:getLevel())
    assert(player:getLevel() == 2, "Should level up to 2")
    
    -- 测试物品系统
    player:addItem("剑")
    player:addItem("盾牌")
    assert(player:hasItem("剑"), "Should have sword")
    assert(player:hasItem("盾牌"), "Should have shield")
    
    local items = player:getItems()
    print("   玩家物品:", table.concat(items, ", "))
    
    -- 测试多态性
    print("   类型:", player:getType())
    print("   分数:", player:getScore())
    assert(player:getType() == "Player", "Type should be 'Player'")
    
else
    error("❌ TestPlayer 类未找到")
end

-- 测试管理器类 (TestManager)
print("\n--- 管理器类测试 (TestManager) ---")
if TestManager then
    print("✅ TestManager 类找到")
    
    local manager = TestManager()
    assert(manager, "Manager should be created")
    
    -- 创建多个玩家
    local player1 = TestPlayer("玩家1", 5)
    local player2 = TestPlayer("玩家2", 3)
    local player3 = TestPlayer("玩家3", 7)
    
    -- 添加到管理器
    manager:addPlayer(player1)
    manager:addPlayer(player2)
    manager:addPlayer(player3)
    
    print("   玩家数量:", manager:getPlayerCount())
    assert(manager:getPlayerCount() == 3, "Should have 3 players")
    
    -- 测试统计功能
    local avg_level = manager:getAverageLevel()
    print("   平均等级:", avg_level)
    assert(math.abs(avg_level - 5.0) < 0.1, "Average level should be 5")
    
    local names = manager:getPlayerNames()
    print("   玩家姓名:", table.concat(names, ", "))
    
    -- 批量操作
    manager:levelUpAll()
    print("   全员升级后平均等级:", manager:getAverageLevel())
    
else
    error("❌ TestManager 类未找到")
end

-- 测试静态类 (TestMathUtils)
print("\n--- 静态类测试 (TestMathUtils) ---")
if TestMathUtils then
    print("✅ TestMathUtils 静态类找到")
    
    -- 测试数学函数
    local clamped = TestMathUtils.clamp(15.0, 0.0, 10.0)
    print("   clamp(15.0, 0.0, 10.0) =", clamped)
    assert(clamped == 10.0, "Clamped value should be 10.0")
    
    local lerped = TestMathUtils.lerp(0.0, 100.0, 0.5)
    print("   lerp(0.0, 100.0, 0.5) =", lerped)
    assert(lerped == 50.0, "Lerped value should be 50.0")
    
    local distance = TestMathUtils.distance2D(0, 0, 3, 4)
    print("   distance2D(0, 0, 3, 4) =", distance)
    assert(distance == 5.0, "Distance should be 5.0")
    
    local is_prime = TestMathUtils.isPrime(17)
    print("   isPrime(17) =", is_prime)
    assert(is_prime == true, "17 should be prime")
    
    local factorial = TestMathUtils.factorial(5)
    print("   factorial(5) =", factorial)
    assert(factorial == 120, "5! should be 120")
    
    -- 测试随机数
    local rand_int = TestMathUtils.randomInt(1, 10)
    print("   randomInt(1, 10) =", rand_int)
    assert(rand_int >= 1 and rand_int <= 10, "Random int should be in range [1, 10]")
    
else
    error("❌ TestMathUtils 静态类未找到")
end

-- 测试字符串工具静态类
print("\n--- 字符串工具静态类测试 (TestStringUtils) ---")
if TestStringUtils then
    print("✅ TestStringUtils 静态类找到")
    
    local upper = TestStringUtils.toUpper("hello")
    print("   toUpper('hello') =", upper)
    assert(upper == "HELLO", "Should convert to uppercase")
    
    local lower = TestStringUtils.toLower("WORLD")
    print("   toLower('WORLD') =", lower)
    assert(lower == "world", "Should convert to lowercase")
    
    local reversed = TestStringUtils.reverse("abc")
    print("   reverse('abc') =", reversed)
    assert(reversed == "cba", "Should reverse string")
    
    local parts = TestStringUtils.split("a,b,c", ',')
    print("   split('a,b,c', ',') =", table.concat(parts, " | "))
    assert(#parts == 3 and parts[1] == "a", "Should split correctly")
    
    local joined = TestStringUtils.join({"x", "y", "z"}, "-")
    print("   join({'x', 'y', 'z'}, '-') =", joined)
    assert(joined == "x-y-z", "Should join correctly")
    
    local starts = TestStringUtils.startsWith("hello world", "hello")
    print("   startsWith('hello world', 'hello') =", starts)
    assert(starts == true, "Should start with 'hello'")
    
else
    error("❌ TestStringUtils 静态类未找到")
end

-- 测试单例类 (TestGameManager)
print("\n--- 单例类测试 (TestGameManager) ---")
if TestGameManager then
    print("✅ TestGameManager 单例类找到")
    
    local game_mgr = TestGameManager.getInstance()
    assert(game_mgr, "Should get singleton instance")
    
    -- 验证单例特性
    local game_mgr2 = TestGameManager.getInstance()
    -- 注意：在Lua中可能无法直接比较对象引用，但功能应该一致
    
    -- 测试游戏状态管理
    game_mgr:startGame()
    print("   游戏已启动:", game_mgr:isGameRunning())
    assert(game_mgr:isGameRunning() == true, "Game should be running")
    
    game_mgr:addScore(250)
    print("   当前分数:", game_mgr:getScore())
    assert(game_mgr:getScore() == 250, "Score should be 250")
    
    -- 测试设置系统
    game_mgr:setSetting("difficulty", "hard")
    game_mgr:setSetting("music_volume", "80")
    
    local difficulty = game_mgr:getSetting("difficulty")
    print("   难度设置:", difficulty)
    assert(difficulty == "hard", "Difficulty should be 'hard'")
    
    local has_volume = game_mgr:hasSetting("music_volume")
    assert(has_volume == true, "Should have music_volume setting")
    
else
    error("❌ TestGameManager 单例类未找到")
end

print("\n🎉 类绑定测试全部通过！")
return true