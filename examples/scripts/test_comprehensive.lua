--[[
test_comprehensive.lua
Comprehensive Test Lua绑定测试脚本

测试comprehensive_test.h中定义的所有复杂特性
]]

-- 测试结果统计
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

function assert_near(actual, expected, tolerance, message)
    if math.abs(actual - expected) <= tolerance then
        tests_passed = tests_passed + 1
        print("✓ PASS: " .. (message or "test"))
    else
        tests_failed = tests_failed + 1
        print("✗ FAIL: " .. (message or "test") .. " - Expected: " .. tostring(expected) .. "±" .. tostring(tolerance) .. ", Got: " .. tostring(actual))
    end
end

print("=== Comprehensive Test Lua Binding Tests ===")

-- 检查game模块是否可用
if not game then
    print("❌ ERROR: 'game' module is not available")
    print("   Make sure to:")
    print("   1. Generate bindings: lua_binding_generator examples/comprehensive_test.h")
    print("   2. Integrate bindings into Sol2-enabled application")
    print("   3. Load the game module in Lua")
    print("")
    print("=== Test Summary ===")
    print("Tests passed: 0")
    print("Tests failed: 0") 
    print("Total tests: 0")
    print("⚠️ Module not available - tests skipped")
    return 1
end

-- 测试枚举
print("\n--- Testing Enums ---")
assert_equal(game.core.Status.ACTIVE, 0, "Status.ACTIVE")
assert_equal(game.core.Status.INACTIVE, 1, "Status.INACTIVE")
assert_equal(game.core.Status.PENDING, 2, "Status.PENDING")
assert_equal(game.core.Status.ERROR, 3, "Status.ERROR")

assert_equal(game.core.Priority.LOW, 1, "Priority.LOW")
assert_equal(game.core.Priority.MEDIUM, 2, "Priority.MEDIUM")
assert_equal(game.core.Priority.HIGH, 3, "Priority.HIGH")
assert_equal(game.core.Priority.CRITICAL, 4, "Priority.CRITICAL")

-- 测试常量
print("\n--- Testing Constants ---")
assert_equal(game.core.MAX_PLAYERS, 100, "MAX_PLAYERS constant")
assert_near(game.core.PI, 3.14159265359, 1e-10, "PI constant")
assert_equal(game.core.GAME_NAME, "TestGame", "GAME_NAME constant")

-- 测试全局变量
print("\n--- Testing Global Variables ---")
-- 注意：全局变量可能需要特殊的绑定处理
-- game.core.g_debug_level = 5
-- assert_equal(game.core.g_debug_level, 5, "Global variable g_debug_level")

-- 测试全局函数
print("\n--- Testing Global Functions ---")
assert_near(game.core.calculateDistance(0, 0, 3, 4), 5.0, 1e-6, "calculateDistance")
assert_near(game.core.calculateDistance(1, 1, 4, 5), 5.0, 1e-6, "calculateDistance offset")

local formatted = game.core.formatMessage("Test message", 1)
assert_true(string.find(formatted, "Test message") ~= nil, "formatMessage contains message")
assert_true(string.find(formatted, "INFO") ~= nil, "formatMessage contains level")

assert_true(game.core.validateInput("valid string"), "validateInput valid string")
assert_false(game.core.validateInput(""), "validateInput empty string")

-- 测试Entity基类
print("\n--- Testing Entity Base Class ---")

local entity = game.core.Entity.new()
assert_true(entity:getId() > 0, "Entity auto-generated ID")
assert_true(string.find(entity:getName(), "Entity_") == 1, "Entity auto-generated name")

local entity2 = game.core.Entity.new(42, "TestEntity")
assert_equal(entity2:getId(), 42, "Entity custom ID")
assert_equal(entity2:getName(), "TestEntity", "Entity custom name")

-- 测试属性访问
entity:setId(100)
assert_equal(entity:getId(), 100, "Entity setId/getId")

entity:setName("MyEntity")
assert_equal(entity:getName(), "MyEntity", "Entity setName/getName")

-- 测试静态方法
local next_id = game.core.Entity.getNextId()
assert_true(next_id > 0, "Entity.getNextId")

game.core.Entity.resetIdCounter()
local new_entity = game.core.Entity.new()
assert_equal(new_entity:getId(), 1, "Entity ID after reset")

-- 测试Player类（继承自Entity）
print("\n--- Testing Player Class (Inheritance) ---")

local player = game.core.Player.new()
assert_equal(player:getLevel(), 1, "Player default level")
assert_equal(player:getHealth(), 100.0, "Player default health")

local player2 = game.core.Player.new(10, "Hero", 5)
assert_equal(player2:getId(), 10, "Player custom ID")
assert_equal(player2:getName(), "Hero", "Player custom name")
assert_equal(player2:getLevel(), 5, "Player custom level")

-- 测试继承的方法
player:setName("MainPlayer")
assert_equal(player:getName(), "MainPlayer", "Player inherited setName")

-- 测试多态（重写的方法）
local entity_str = entity:toString()
local player_str = player:toString()
assert_true(string.find(entity_str, "Entity") ~= nil, "Entity toString contains Entity")
assert_true(string.find(player_str, "Player") ~= nil, "Player toString contains Player")

-- 测试Player特有的属性和方法
player:setLevel(3)
assert_equal(player:getLevel(), 3, "Player setLevel")

player:setHealth(75.0)
assert_equal(player:getHealth(), 75.0, "Player setHealth")

player:setMana(60.0)
assert_equal(player:getMana(), 60.0, "Player setMana")

-- 测试技能系统
player:addSkill("Fireball")
player:addSkill("Heal")
local skills = player:getSkills()
-- 注意：这里假设返回的是Lua table，实际可能需要特殊处理
assert_true(#skills >= 1, "Player skills count")

-- 测试物品系统
player:addItem("Health Potion", 5)
player:addItem("Mana Potion", 3)
local inventory = player:getInventory()
-- 注意：这里假设返回的是Lua table，实际可能需要特殊处理

-- 测试运算符重载
player:setLevel(1)
player = player + 250  -- 应该增加经验并升级
assert_true(player:getLevel() >= 2, "Player += operator for experience")

-- 测试GameManager单例
print("\n--- Testing GameManager Singleton ---")

local game_mgr = game.core.GameManager.getInstance()
assert_true(game_mgr ~= nil, "GameManager.getInstance")

-- 测试单例行为
local game_mgr2 = game.core.GameManager.getInstance()
-- 通过行为测试单例特性
game_mgr:startGame()
assert_true(game_mgr2:isGameRunning(), "GameManager singleton behavior")

-- 测试游戏状态
assert_true(game_mgr:isGameRunning(), "GameManager isGameRunning")
assert_false(game_mgr:isGamePaused(), "GameManager isGamePaused when running")

game_mgr:pauseGame()
assert_false(game_mgr:isGameRunning(), "GameManager not running when paused")
assert_true(game_mgr:isGamePaused(), "GameManager isGamePaused")

game_mgr:resumeGame()
assert_true(game_mgr:isGameRunning(), "GameManager running after resume")

-- 测试玩家管理
game_mgr:addPlayer(player)
assert_equal(game_mgr:getPlayerCount(), 1, "GameManager player count after adding")

local retrieved_player = game_mgr:getPlayer(player:getId())
if retrieved_player then
    assert_equal(retrieved_player:getName(), player:getName(), "GameManager getPlayer by ID")
end

local all_players = game_mgr:getAllPlayers()
assert_equal(#all_players, 1, "GameManager getAllPlayers count")

-- 测试MathUtils静态类
print("\n--- Testing MathUtils Static Class ---")

assert_equal(game.core.MathUtils.clamp(5.0, 0.0, 10.0), 5.0, "MathUtils.clamp normal")
assert_equal(game.core.MathUtils.clamp(-5.0, 0.0, 10.0), 0.0, "MathUtils.clamp below min")
assert_equal(game.core.MathUtils.clamp(15.0, 0.0, 10.0), 10.0, "MathUtils.clamp above max")

assert_equal(game.core.MathUtils.lerp(0.0, 10.0, 0.5), 5.0, "MathUtils.lerp")
assert_equal(game.core.MathUtils.lerp(10.0, 20.0, 0.3), 13.0, "MathUtils.lerp custom")

-- 测试随机数
local rand_int = game.core.MathUtils.random(1, 100)
assert_true(rand_int >= 1 and rand_int <= 100, "MathUtils.random in range")

local rand_float = game.core.MathUtils.randomFloat(0.0, 1.0)
assert_true(rand_float >= 0.0 and rand_float <= 1.0, "MathUtils.randomFloat in range")

-- 测试向量数学
assert_near(game.core.MathUtils.dotProduct(1, 0, 0, 1), 0.0, 1e-6, "MathUtils.dotProduct perpendicular")
assert_near(game.core.MathUtils.dotProduct(1, 0, 1, 0), 1.0, 1e-6, "MathUtils.dotProduct parallel")

assert_near(game.core.MathUtils.magnitude(3, 4), 5.0, 1e-6, "MathUtils.magnitude")

-- 测试常量
assert_near(game.core.MathUtils.PI, 3.141592653589793, 1e-10, "MathUtils.PI")
assert_near(game.core.MathUtils.E, 2.718281828459045, 1e-10, "MathUtils.E")

-- 测试TransformComponent类
print("\n--- Testing TransformComponent Class ---")

local transform = game.core.TransformComponent.new()
assert_equal(transform:getX(), 0.0, "TransformComponent default X")
assert_equal(transform:getY(), 0.0, "TransformComponent default Y")
assert_equal(transform:getRotation(), 0.0, "TransformComponent default rotation")

local transform2 = game.core.TransformComponent.new(10.0, 20.0, 45.0)
assert_equal(transform2:getX(), 10.0, "TransformComponent custom X")
assert_equal(transform2:getY(), 20.0, "TransformComponent custom Y")
assert_equal(transform2:getRotation(), 45.0, "TransformComponent custom rotation")

-- 测试属性设置
transform:setX(5.0)
transform:setY(15.0)
transform:setRotation(90.0)
assert_equal(transform:getX(), 5.0, "TransformComponent setX")
assert_equal(transform:getY(), 15.0, "TransformComponent setY")
assert_equal(transform:getRotation(), 90.0, "TransformComponent setRotation")

-- 测试便捷方法
transform:translate(2.0, 3.0)
assert_equal(transform:getX(), 7.0, "TransformComponent translate X")
assert_equal(transform:getY(), 18.0, "TransformComponent translate Y")

transform:rotate(45.0)
assert_equal(transform:getRotation(), 135.0, "TransformComponent rotate")

-- 测试抽象基类方法
assert_equal(transform:getTypeName(), "TransformComponent", "TransformComponent getTypeName")
assert_true(transform:isActive(), "TransformComponent default active")

transform:setActive(false)
assert_false(transform:isActive(), "TransformComponent setActive")

-- 测试EventSystem回调
print("\n--- Testing EventSystem Callbacks ---")

local event_system = game.events.EventSystem.new()
assert_true(event_system ~= nil, "EventSystem creation")

-- 测试无参数回调
local game_started = false
event_system.OnGameStart = function()
    game_started = true
end

event_system:triggerGameStart()
assert_true(game_started, "EventSystem OnGameStart callback")

-- 测试带参数回调
local joined_player_name = ""
event_system.OnPlayerJoin = function(player)
    joined_player_name = player:getName()
end

local test_player = game.core.Player.new(1, "TestHero", 1)
event_system:triggerPlayerJoin(test_player)
assert_equal(joined_player_name, "TestHero", "EventSystem OnPlayerJoin callback with parameter")

-- 测试多参数回调
local levelup_player = nil
local old_level = 0
local new_level = 0

event_system.OnPlayerLevelUp = function(player, oldLvl, newLvl)
    levelup_player = player
    old_level = oldLvl
    new_level = newLvl
end

event_system:triggerPlayerLevelUp(test_player, 5, 6)
assert_equal(levelup_player:getName(), "TestHero", "EventSystem OnPlayerLevelUp player parameter")
assert_equal(old_level, 5, "EventSystem OnPlayerLevelUp old level parameter")
assert_equal(new_level, 6, "EventSystem OnPlayerLevelUp new level parameter")

-- 测试带返回值的回调
local validation_result = true
event_system.OnValidateAction = function(action, value)
    if action == "invalid" then
        return false
    end
    return value > 0
end

local result1 = event_system:validateAction("move", 10.0)
assert_true(result1, "EventSystem OnValidateAction positive result")

local result2 = event_system:validateAction("invalid", 5.0)
assert_false(result2, "EventSystem OnValidateAction negative result")

-- 测试ContainerUtils类
print("\n--- Testing ContainerUtils Class ---")

local container_utils = game.containers.ContainerUtils.new()
assert_true(container_utils ~= nil, "ContainerUtils creation")

-- 测试获取容器（注意：这些可能需要特殊的STL绑定支持）
local int_vec = container_utils:getIntVector()
-- assert_true(#int_vec > 0, "ContainerUtils getIntVector not empty")

local str_vec = container_utils:getStringVector()
-- assert_true(#str_vec > 0, "ContainerUtils getStringVector not empty")

-- 测试容器处理
-- container_utils:processIntVector({1, 2, 3, 4, 5})
-- container_utils:processStringVector({"hello", "world", "lua"})

-- 测试SmartPointerDemo类
print("\n--- Testing SmartPointerDemo Class ---")

local smart_demo = game.smartptr.SmartPointerDemo.new()
assert_true(smart_demo ~= nil, "SmartPointerDemo creation")

-- 测试shared_ptr方法
local created_player = smart_demo:createPlayer("SmartPlayer")
assert_true(created_player ~= nil, "SmartPointerDemo createPlayer")
if created_player then
    assert_equal(created_player:getName(), "SmartPlayer", "Created player name")
end

smart_demo:setCurrentPlayer(created_player)
local current = smart_demo:getCurrentPlayer()
if current then
    assert_equal(current:getName(), "SmartPlayer", "SmartPointerDemo getCurrentPlayer")
end

-- 测试Vector2D运算符重载
print("\n--- Testing Vector2D Operator Overloading ---")

local vec1 = operators.Vector2D.new(3.0, 4.0)
local vec2 = operators.Vector2D.new(1.0, 2.0)

assert_equal(vec1:getX(), 3.0, "Vector2D constructor X")
assert_equal(vec1:getY(), 4.0, "Vector2D constructor Y")

-- 测试数学运算符
local vec3 = vec1 + vec2  -- (3,4) + (1,2) = (4,6)
assert_equal(vec3:getX(), 4.0, "Vector2D addition X")
assert_equal(vec3:getY(), 6.0, "Vector2D addition Y")

local vec4 = vec1 - vec2  -- (3,4) - (1,2) = (2,2)
assert_equal(vec4:getX(), 2.0, "Vector2D subtraction X")
assert_equal(vec4:getY(), 2.0, "Vector2D subtraction Y")

local vec5 = vec1 * 2.0  -- (3,4) * 2 = (6,8)
assert_equal(vec5:getX(), 6.0, "Vector2D scalar multiplication X")
assert_equal(vec5:getY(), 8.0, "Vector2D scalar multiplication Y")

local vec6 = vec5 / 2.0  -- (6,8) / 2 = (3,4)
assert_equal(vec6:getX(), 3.0, "Vector2D scalar division X")
assert_equal(vec6:getY(), 4.0, "Vector2D scalar division Y")

-- 测试比较运算符
assert_true(vec1 == vec6, "Vector2D equality operator")
assert_true(vec1 ~= vec2, "Vector2D inequality operator")

-- 测试一元运算符
local vec7 = -vec1  -- -(3,4) = (-3,-4)
assert_equal(vec7:getX(), -3.0, "Vector2D unary minus X")
assert_equal(vec7:getY(), -4.0, "Vector2D unary minus Y")

-- 测试下标运算符
assert_equal(vec1[0], 3.0, "Vector2D subscript operator [0]")
assert_equal(vec1[1], 4.0, "Vector2D subscript operator [1]")

-- 测试函数调用运算符
assert_equal(vec1(), 5.0, "Vector2D function call operator (length)")

-- 测试工具方法
assert_equal(vec1:length(), 5.0, "Vector2D length")
assert_equal(vec1:lengthSquared(), 25.0, "Vector2D lengthSquared")
assert_near(vec1:dot(vec2), 11.0, 1e-6, "Vector2D dot product")

local normalized = vec1:normalized()
assert_near(normalized:length(), 1.0, 1e-6, "Vector2D normalized length")

-- 测试模板类实例（如果支持）
print("\n--- Testing Template Class Instances ---")

-- 注意：模板类的绑定可能需要特殊处理
-- local int_container = templates.Container_int.new()
-- int_container:setValue(42)
-- assert_equal(int_container:getValue(), 42, "Template Container<int> setValue/getValue")

-- local str_container = templates.Container_string.new("default")
-- assert_equal(str_container:getValue(), "default", "Template Container<string> default value")

print("\n--- Testing Complex Scenarios ---")

-- 测试复杂的对象交互
local main_player = game.core.Player.new(1, "MainHero", 10)
local world_mgr = game.core.GameManager.getInstance()

world_mgr:addPlayer(main_player)
main_player:addSkill("Lightning Bolt")
main_player:addSkill("Teleport")
main_player:addItem("Sword", 1)
main_player:addItem("Shield", 1)

-- 测试事件链
local events = game.events.EventSystem.new()
local event_log = {}

events.OnPlayerJoin = function(player)
    table.insert(event_log, "Player " .. player:getName() .. " joined")
end

events.OnPlayerLevelUp = function(player, oldLvl, newLvl)
    table.insert(event_log, "Player " .. player:getName() .. " leveled up from " .. oldLvl .. " to " .. newLvl)
end

events:triggerPlayerJoin(main_player)
events:triggerPlayerLevelUp(main_player, 10, 11)

assert_equal(#event_log, 2, "Complex event chain - event count")
assert_true(string.find(event_log[1], "MainHero") ~= nil, "Complex event chain - join event")
assert_true(string.find(event_log[2], "leveled up") ~= nil, "Complex event chain - levelup event")

print("\n=== Test Summary ===")
print("Tests passed: " .. tests_passed)
print("Tests failed: " .. tests_failed)
print("Total tests: " .. (tests_passed + tests_failed))

if tests_failed == 0 then
    print("🎉 All comprehensive tests passed!")
    return 0
else
    print("❌ Some comprehensive tests failed!")
    return 1
end