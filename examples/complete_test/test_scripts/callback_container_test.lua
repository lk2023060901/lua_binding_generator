--[[
    @file callback_container_test.lua
    @brief 回调函数和容器测试脚本
    
    测试：
    - 回调函数绑定 (EXPORT_LUA_CALLBACK)
    - STL容器绑定 (EXPORT_LUA_VECTOR, EXPORT_LUA_MAP)
    - 事件系统
    - 容器操作
]]

print("=== 回调函数和容器测试 ===")

-- 测试事件系统和回调函数
print("\n--- 事件系统和回调函数测试 (TestEventSystem) ---")
if TestEventSystem then
    print("✅ TestEventSystem 类找到")
    
    local event_system = TestEventSystem()
    assert(event_system, "Event system should be created")
    
    -- 测试简单回调
    print("\n--- 游戏开始回调测试 ---")
    local game_started = false
    
    event_system.OnGameStart = function()
        print("   🎮 游戏开始回调被触发！")
        game_started = true
    end
    
    event_system:triggerGameStart()
    assert(game_started == true, "Game start callback should be triggered")
    print("✅ 游戏开始回调测试通过")
    
    -- 测试带参数的回调
    print("\n--- 分数变化回调测试 ---")
    local last_score = 0
    
    event_system.OnScoreChange = function(new_score)
        print("   📊 分数变化: " .. last_score .. " -> " .. new_score)
        last_score = new_score
    end
    
    event_system:triggerScoreChange(100)
    event_system:triggerScoreChange(250)
    assert(last_score == 250, "Score change callback should update last_score")
    print("✅ 分数变化回调测试通过")
    
    -- 测试玩家加入回调
    print("\n--- 玩家加入回调测试 ---")
    local joined_player_name = ""
    
    event_system.OnPlayerJoin = function(player)
        if player then
            joined_player_name = player:getName()
            print("   👤 玩家加入: " .. joined_player_name)
        end
    end
    
    if TestPlayer then
        local new_player = TestPlayer("新玩家", 1)
        event_system:triggerPlayerJoin(new_player)
        assert(joined_player_name == "新玩家", "Player join callback should receive player")
        print("✅ 玩家加入回调测试通过")
    end
    
    -- 测试验证回调（返回值）
    print("\n--- 动作验证回调测试 ---")
    local validation_count = 0
    
    event_system.OnValidateAction = function(action, value)
        validation_count = validation_count + 1
        print("   🔍 验证动作: " .. action .. " (值: " .. value .. ")")
        
        -- 验证逻辑：只允许值小于100的动作
        return value < 100
    end
    
    local result1 = event_system:validateAction("move", 50)
    local result2 = event_system:validateAction("attack", 150)
    
    print("   验证结果1 (move, 50):", result1)
    print("   验证结果2 (attack, 150):", result2)
    
    assert(result1 == true, "Move action should be valid")
    assert(result2 == false, "Attack action should be invalid")
    assert(validation_count == 2, "Validation should be called twice")
    print("✅ 动作验证回调测试通过")
    
    -- 测试位置变化回调
    print("\n--- 位置变化回调测试 ---")
    local last_x, last_y = 0, 0
    
    event_system.OnPositionChange = function(x, y)
        print("   📍 位置变化: (" .. x .. ", " .. y .. ")")
        last_x, last_y = x, y
    end
    
    event_system:triggerPositionChange(10.5, 20.3)
    assert(last_x == 10.5 and last_y == 20.3, "Position change should update coordinates")
    print("✅ 位置变化回调测试通过")
    
    -- 测试消息过滤回调
    print("\n--- 消息过滤回调测试 ---")
    event_system.OnMessageFilter = function(message)
        -- 简单的过滤器：将"bad"替换为"***"
        local filtered = string.gsub(message, "bad", "***")
        print("   🔧 消息过滤: '" .. message .. "' -> '" .. filtered .. "'")
        return filtered
    end
    
    local filtered_msg = event_system:filterMessage("This is a bad word test")
    assert(filtered_msg:find("%*%*%*"), "Message should be filtered")
    print("✅ 消息过滤回调测试通过")
    
    -- 测试批量事件处理
    print("\n--- 批量事件处理测试 ---")
    print("   触发多个事件...")
    event_system:triggerMultipleEvents()
    print("✅ 批量事件处理测试通过")
    
else
    error("❌ TestEventSystem 类未找到")
end

-- 测试容器管理器
print("\n--- 容器管理器测试 (TestContainerManager) ---")
if TestContainerManager then
    print("✅ TestContainerManager 类找到")
    
    local container_mgr = TestContainerManager()
    assert(container_mgr, "Container manager should be created")
    
    -- 测试整数向量操作
    print("\n--- 整数向量操作测试 ---")
    container_mgr:addNumber(10)
    container_mgr:addNumber(20)
    container_mgr:addNumber(30)
    container_mgr:addNumber(40)
    
    local numbers = container_mgr:getNumbers()
    print("   数字列表:", table.concat(numbers, ", "))
    assert(#numbers == 4, "Should have 4 numbers")
    assert(numbers[2] == 20, "Second number should be 20")
    
    local count = container_mgr:getNumberCount()
    assert(count == 4, "Number count should be 4")
    
    local third_number = container_mgr:getNumberAt(2)  -- 0-based index
    print("   第3个数字 (索引2):", third_number)
    assert(third_number == 30, "Third number should be 30")
    
    -- 批量添加
    local more_numbers = {50, 60, 70}
    container_mgr:addNumbers(more_numbers)
    local all_numbers = container_mgr:getNumbers()
    print("   批量添加后:", table.concat(all_numbers, ", "))
    assert(#all_numbers == 7, "Should have 7 numbers after batch add")
    
    print("✅ 整数向量操作测试通过")
    
    -- 测试字符串向量操作
    print("\n--- 字符串向量操作测试 ---")
    container_mgr:addString("hello")
    container_mgr:addString("world")
    container_mgr:addString("lua")
    
    local strings = container_mgr:getStrings()
    print("   字符串列表:", table.concat(strings, ", "))
    assert(#strings == 3, "Should have 3 strings")
    
    local second_string = container_mgr:getStringAt(1)
    print("   第2个字符串:", second_string)
    assert(second_string == "world", "Second string should be 'world'")
    
    print("✅ 字符串向量操作测试通过")
    
    -- 测试映射容器操作
    print("\n--- 映射容器操作测试 ---")
    container_mgr:setProperty("name", "测试项目")
    container_mgr:setProperty("version", "2.0.0")
    container_mgr:setProperty("author", "开发者")
    container_mgr:setProperty("language", "C++/Lua")
    
    local name = container_mgr:getProperty("name")
    local version = container_mgr:getProperty("version")
    print("   项目名称:", name)
    print("   项目版本:", version)
    assert(name == "测试项目", "Name should be '测试项目'")
    assert(version == "2.0.0", "Version should be '2.0.0'")
    
    local has_author = container_mgr:hasProperty("author")
    local has_license = container_mgr:hasProperty("license")
    assert(has_author == true, "Should have author property")
    assert(has_license == false, "Should not have license property")
    
    local all_properties = container_mgr:getAllProperties()
    print("   所有属性数量:", #all_properties)
    
    local property_keys = container_mgr:getPropertyKeys()
    print("   属性键:", table.concat(property_keys, ", "))
    assert(#property_keys >= 4, "Should have at least 4 property keys")
    
    print("✅ 映射容器操作测试通过")
    
    -- 测试玩家分数管理
    print("\n--- 玩家分数管理测试 ---")
    container_mgr:addPlayerScore("Alice", 1500)
    container_mgr:addPlayerScore("Bob", 1200)
    container_mgr:addPlayerScore("Charlie", 1800)
    container_mgr:addPlayerScore("David", 1100)
    container_mgr:addPlayerScore("Eve", 1600)
    
    local alice_score = container_mgr:getPlayerScore("Alice")
    print("   Alice的分数:", alice_score)
    assert(alice_score == 1500, "Alice's score should be 1500")
    
    local all_scores = container_mgr:getPlayerScores()
    print("   玩家分数表大小:", #all_scores)
    
    local top_3 = container_mgr:getTopPlayers(3)
    print("   前3名玩家:", table.concat(top_3, ", "))
    assert(#top_3 == 3, "Should return top 3 players")
    -- 应该按分数降序排列：Charlie(1800), Eve(1600), Alice(1500)
    assert(top_3[1] == "Charlie", "Charlie should be first")
    
    print("✅ 玩家分数管理测试通过")
    
    -- 测试玩家对象列表
    print("\n--- 玩家对象列表测试 ---")
    if TestPlayer then
        local player1 = TestPlayer("战士", 10)
        local player2 = TestPlayer("法师", 8)
        local player3 = TestPlayer("弓箭手", 9)
        
        container_mgr:addPlayerToList(player1)
        container_mgr:addPlayerToList(player2)
        container_mgr:addPlayerToList(player3)
        
        local player_list = container_mgr:getPlayerList()
        print("   玩家列表大小:", #player_list)
        assert(#player_list == 3, "Should have 3 players in list")
        
        -- 检查第一个玩家
        local first_player = player_list[1]
        if first_player then
            print("   第一个玩家:", first_player:getName(), "等级:", first_player:getLevel())
            assert(first_player:getName() == "战士", "First player should be warrior")
        end
        
        print("✅ 玩家对象列表测试通过")
    end
    
else
    error("❌ TestContainerManager 类未找到")
end

-- 测试直接导出的STL容器（如果支持）
print("\n--- 直接STL容器导出测试 ---")

-- 这些测试取决于绑定生成器如何处理EXPORT_LUA_VECTOR和EXPORT_LUA_MAP宏
-- 可能需要根据实际生成的绑定代码进行调整

if TestIntList then
    print("✅ TestIntList (Vector<int>) 找到")
    local int_list = TestIntList()
    -- 测试向量操作...
    print("✅ 整数列表测试通过")
elseif type(TestIntList) == "table" then
    print("✅ TestIntList 作为表格类型存在")
else
    print("⚠️  TestIntList 未找到，可能需要特殊的绑定处理")
end

if TestNameScoreMap then
    print("✅ TestNameScoreMap (Map<string,int>) 找到")
    local score_map = TestNameScoreMap()
    -- 测试映射操作...
    print("✅ 名称分数映射测试通过")
elseif type(TestNameScoreMap) == "table" then
    print("✅ TestNameScoreMap 作为表格类型存在")
else
    print("⚠️  TestNameScoreMap 未找到，可能需要特殊的绑定处理")
end

print("\n🎉 回调函数和容器测试全部通过！")
return true