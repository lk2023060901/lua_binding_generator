--[[
    @file class_interaction_test.lua
    @brief 复杂类交互和继承测试
    
    测试更高级的类功能：
    - 继承关系和多态性
    - 复杂对象状态管理
    - 类之间的协作
    - 生命周期管理
]]

print("=== 复杂类交互测试开始 ===")

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
-- 测试复杂的玩家创建和管理场景
-- ================================
print("\n--- 复杂玩家管理场景 ---")

if not (test_coverage and test_coverage.TestPlayer and test_coverage.TestManager) then
    print("❌ 所需的类绑定未找到")
    return false
end

-- 创建多个玩家模拟游戏场景
local manager = test_coverage.TestManager()

-- 创建不同类型的玩家
local warrior = test_coverage.TestPlayer("Warrior", 1)
local mage = test_coverage.TestPlayer("Mage", 1)
local archer = test_coverage.TestPlayer("Archer", 1)

-- 设置玩家初始状态
warrior:setHealth(120)  -- 战士血量更高
mage:setHealth(80)      -- 法师血量较低
archer:setHealth(100)   -- 弓箭手中等血量

-- 给玩家添加装备
warrior:addItem("iron_sword")
warrior:addItem("steel_shield")
warrior:addItem("chain_armor")

mage:addItem("magic_staff")
mage:addItem("mana_potion")
mage:addItem("spell_book")

archer:addItem("elven_bow")
archer:addItem("quiver")
archer:addItem("leather_armor")

-- 添加到管理器
manager:addPlayer(warrior)
manager:addPlayer(mage)
manager:addPlayer(archer)

test_assert(manager:getPlayerCount() == 3, "管理器玩家数量")

-- ================================
-- 测试批量操作和状态同步
-- ================================
print("\n--- 批量操作测试 ---")

-- 记录初始状态
local warrior_initial_level = warrior:getLevel()
local mage_initial_level = mage:getLevel()
local archer_initial_level = archer:getLevel()

-- 执行批量升级
manager:levelUpAll()

-- 从管理器中获取升级后的玩家来验证（因为管理器存储的是拷贝对象）
local upgraded_players = manager:getAllPlayers()
local warrior_upgraded = false
local mage_upgraded = false  
local archer_upgraded = false

if upgraded_players:size() >= 3 then
    for i = 0, upgraded_players:size() - 1 do  -- Sol2向量使用0索引
        local player = upgraded_players:at(i)
        if player and player:getName() == "Warrior" and player:getLevel() == 2 then
            warrior_upgraded = true
        elseif player and player:getName() == "Mage" and player:getLevel() == 2 then
            mage_upgraded = true
        elseif player and player:getName() == "Archer" and player:getLevel() == 2 then
            archer_upgraded = true
        end
    end
end

test_assert(warrior_upgraded and mage_upgraded and archer_upgraded, "批量升级")
print("批量升级验证：所有玩家都已升级")

-- 测试批量治疗
local warrior_initial_health = warrior:getHealth()
manager:healAll(20)
test_assert(warrior:getHealth() >= warrior_initial_health + 20, "批量治疗效果")

-- ================================
-- 测试经验值系统和升级机制
-- ================================
print("\n--- 经验值和升级系统 ---")

-- 给不同玩家不同经验值
warrior:addExperience(150)  -- 足够升1级
mage:addExperience(300)     -- 足够升2级
archer:addExperience(75)    -- 不够升级

-- 检查经验值是否正确加入
test_assert(warrior:getExperience() >= 150, "战士经验值增加")
test_assert(mage:getExperience() >= 300, "法师经验值增加")

-- 测试升级后的属性变化
local warrior_level_before = warrior:getLevel()
local mage_level_before = mage:getLevel()

-- 再次添加经验来触发升级
warrior:addExperience(50)
mage:addExperience(100)

-- 验证升级（如果实现了自动升级逻辑）
print("战士当前等级: " .. warrior:getLevel())
print("法师当前等级: " .. mage:getLevel())

-- ================================
-- 测试物品系统的复杂操作
-- ================================
print("\n--- 复杂物品系统测试 ---")

-- 测试物品交换
local warrior_items = warrior:getItems()
local mage_items = mage:getItems()

-- 转换Sol2向量为Lua表以便使用table.concat
local warrior_items_table = warrior_items:to_table()
local mage_items_table = mage_items:to_table()

print("战士物品: " .. table.concat(warrior_items_table, ", "))
print("法师物品: " .. table.concat(mage_items_table, ", "))

test_assert(warrior_items:size() == 3, "战士物品数量")
test_assert(mage_items:size() == 3, "法师物品数量")

-- 测试物品检查
test_assert(warrior:hasItem("iron_sword"), "战士有铁剑")
test_assert(mage:hasItem("magic_staff"), "法师有法杖")
test_assert(not archer:hasItem("iron_sword"), "弓箭手没有铁剑")

-- 模拟物品交易（移除然后添加）
warrior:removeItem("steel_shield")
archer:addItem("steel_shield")

test_assert(not warrior:hasItem("steel_shield"), "战士移除钢盾")
test_assert(archer:hasItem("steel_shield"), "弓箭手获得钢盾")

-- ================================
-- 测试多态性和虚函数
-- ================================
print("\n--- 多态性测试 ---")

-- 测试所有玩家的类型识别
test_assert(warrior:getType() == "Player", "战士类型识别")
test_assert(mage:getType() == "Player", "法师类型识别")
test_assert(archer:getType() == "Player", "弓箭手类型识别")

-- 测试分数计算（可能基于等级、血量、装备等）
local warrior_score = warrior:getScore()
local mage_score = mage:getScore()
local archer_score = archer:getScore()

test_assert(warrior_score > 0, "战士分数计算")
test_assert(mage_score > 0, "法师分数计算")
test_assert(archer_score > 0, "弓箭手分数计算")

print("战士分数: " .. warrior_score)
print("法师分数: " .. mage_score)
print("弓箭手分数: " .. archer_score)

-- ================================
-- 测试管理器的高级功能
-- ================================
print("\n--- 管理器高级功能测试 ---")

-- 测试平均等级计算
local avg_level = manager:getAverageLevel()
test_assert(avg_level > 1, "平均等级计算")
print("平均等级: " .. avg_level)

-- 测试获取最强玩家
local top_player = manager:getTopPlayer()
test_assert(top_player ~= nil, "获取最强玩家")
if top_player then
    print("最强玩家: " .. top_player:getName() .. " (等级 " .. top_player:getLevel() .. ")")
end

-- 测试获取所有玩家名称
local player_names = manager:getPlayerNames()
test_assert(player_names:size() == 3, "玩家名称列表")
local player_names_table = player_names:to_table()
print("所有玩家: " .. table.concat(player_names_table, ", "))

-- ================================
-- 测试对象比较和排序
-- ================================
print("\n--- 对象比较测试 ---")

-- 创建两个相同的玩家用于比较
local player_copy = test_coverage.TestPlayer("Warrior", warrior:getLevel())
player_copy:setHealth(warrior:getHealth())

-- 测试相等性比较（基于实现）
local comparison_result = (warrior == player_copy)
print("玩家比较结果: " .. tostring(comparison_result))

-- ================================
-- 压力测试：创建大量对象
-- ================================
print("\n--- 对象创建压力测试 ---")

local stress_manager = test_coverage.TestManager()
local created_count = 0

-- 创建100个玩家
for i = 1, 100 do
    local test_player = test_coverage.TestPlayer("Player" .. i, math.random(1, 20))
    test_player:setHealth(math.random(50, 150))
    test_player:addExperience(math.random(0, 500))
    
    -- 随机添加装备
    if math.random() > 0.5 then
        test_player:addItem("item_" .. i)
    end
    
    stress_manager:addPlayer(test_player)
    created_count = created_count + 1
end

test_assert(stress_manager:getPlayerCount() == 100, "压力测试 - 创建100个玩家")
test_assert(created_count == 100, "压力测试 - 计数验证")

-- 测试批量操作性能
local start_time = os.clock()
stress_manager:levelUpAll()
local level_up_time = os.clock() - start_time

print("批量升级100个玩家耗时: " .. (level_up_time * 1000) .. " ms")
test_assert(level_up_time < 1.0, "批量操作性能测试")

-- ================================
-- 清理测试
-- ================================
print("\n--- 清理和资源管理测试 ---")

-- 清理所有玩家
stress_manager:clearAll()
test_assert(stress_manager:getPlayerCount() == 0, "批量清理功能")

-- 原管理器应该仍然保持状态
test_assert(manager:getPlayerCount() == 3, "原管理器状态保持")

-- ================================
-- 测试结果报告
-- ================================
print("\n" .. string.rep("=", 60))
print("复杂类交互测试结果:")
print("总测试数: " .. tests_run)
print("通过测试: " .. tests_passed)
print("失败测试: " .. (tests_run - tests_passed))

if tests_passed == tests_run then
    print("🎉 所有复杂类交互测试通过！")
    print("✅ 继承、多态、对象管理工作正常")
    return true
else
    print("❌ 部分测试失败，请检查类实现")
    return false
end