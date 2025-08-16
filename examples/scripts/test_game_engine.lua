--[[
test_game_engine.lua
Game Engine Example Lua绑定测试脚本

测试game_engine.h中定义的所有导出项
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

print("=== Game Engine Lua Binding Tests ===")

-- 检查engine模块是否可用
if not engine then
    print("❌ ERROR: 'engine' module is not available")
    print("   Make sure to:")
    print("   1. Generate bindings: lua_binding_generator examples/game_engine.h")
    print("   2. Integrate bindings into Sol2-enabled application")
    print("   3. Load the engine module in Lua")
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
assert_equal(engine.core.ObjectType.UNKNOWN, 0, "ObjectType.UNKNOWN")
assert_equal(engine.core.ObjectType.PLAYER, 1, "ObjectType.PLAYER")
assert_equal(engine.core.ObjectType.ENEMY, 2, "ObjectType.ENEMY")
assert_equal(engine.core.ObjectType.ITEM, 3, "ObjectType.ITEM")

assert_equal(engine.core.GameState.MENU, 0, "GameState.MENU")
assert_equal(engine.core.GameState.LOADING, 1, "GameState.LOADING")
assert_equal(engine.core.GameState.PLAYING, 2, "GameState.PLAYING")
assert_equal(engine.core.GameState.PAUSED, 3, "GameState.PAUSED")
assert_equal(engine.core.GameState.GAME_OVER, 4, "GameState.GAME_OVER")

-- 测试常量
print("\n--- Testing Constants ---")
assert_equal(engine.core.MAX_ENTITIES, 1000, "MAX_ENTITIES constant")
assert_equal(engine.core.WORLD_WIDTH, 1920.0, "WORLD_WIDTH constant")
assert_equal(engine.core.WORLD_HEIGHT, 1080.0, "WORLD_HEIGHT constant")
assert_equal(engine.core.ENGINE_VERSION, "1.0.0", "ENGINE_VERSION constant")

-- 测试Time类
print("\n--- Testing Time Class ---")
local time = engine.core.Time.new()
assert_equal(time:getDeltaTime(), 0.0, "Time initial delta time")
assert_equal(time:getTotalTime(), 0.0, "Time initial total time")
assert_equal(time:getFrameCount(), 0, "Time initial frame count")
assert_equal(time:getTimeScale(), 1.0, "Time initial time scale")

time:setTimeScale(2.0)
assert_equal(time:getTimeScale(), 2.0, "Time setTimeScale")

-- 测试静态方法
local current_time = engine.core.Time.getCurrentTime()
assert_true(current_time > 0, "Time.getCurrentTime returns positive value")

-- 测试Vector2类
print("\n--- Testing Vector2 Class ---")

-- 测试构造函数
local vec1 = engine.core.Vector2.new()
assert_equal(vec1:getX(), 0.0, "Vector2 default constructor X")
assert_equal(vec1:getY(), 0.0, "Vector2 default constructor Y")

local vec2 = engine.core.Vector2.new(3.0, 4.0)
assert_equal(vec2:getX(), 3.0, "Vector2 parameterized constructor X")
assert_equal(vec2:getY(), 4.0, "Vector2 parameterized constructor Y")

-- 测试属性
vec1:setX(1.0)
vec1:setY(2.0)
assert_equal(vec1:getX(), 1.0, "Vector2 setX")
assert_equal(vec1:getY(), 2.0, "Vector2 setY")

-- 测试运算符重载
local vec3 = vec1 + vec2  -- (1,2) + (3,4) = (4,6)
assert_equal(vec3:getX(), 4.0, "Vector2 addition X")
assert_equal(vec3:getY(), 6.0, "Vector2 addition Y")

local vec4 = vec2 - vec1  -- (3,4) - (1,2) = (2,2)
assert_equal(vec4:getX(), 2.0, "Vector2 subtraction X")
assert_equal(vec4:getY(), 2.0, "Vector2 subtraction Y")

local vec5 = vec2 * 2.0  -- (3,4) * 2 = (6,8)
assert_equal(vec5:getX(), 6.0, "Vector2 scalar multiplication X")
assert_equal(vec5:getY(), 8.0, "Vector2 scalar multiplication Y")

local vec6 = vec5 / 2.0  -- (6,8) / 2 = (3,4)
assert_equal(vec6:getX(), 3.0, "Vector2 scalar division X")
assert_equal(vec6:getY(), 4.0, "Vector2 scalar division Y")

assert_true(vec2 == vec6, "Vector2 equality operator")

-- 测试向量数学
assert_equal(vec2:length(), 5.0, "Vector2 length (3,4) = 5")
assert_equal(vec2:lengthSquared(), 25.0, "Vector2 lengthSquared")
assert_near(vec2:dot(vec1), 11.0, 1e-6, "Vector2 dot product (3,4)·(1,2) = 11")

local normalized = vec2:normalized()
assert_near(normalized:length(), 1.0, 1e-6, "Vector2 normalized length")

-- 测试静态方法
local zero = engine.core.Vector2.zero()
assert_equal(zero:getX(), 0.0, "Vector2.zero() X")
assert_equal(zero:getY(), 0.0, "Vector2.zero() Y")

local one = engine.core.Vector2.one()
assert_equal(one:getX(), 1.0, "Vector2.one() X")
assert_equal(one:getY(), 1.0, "Vector2.one() Y")

local up = engine.core.Vector2.up()
assert_equal(up:getX(), 0.0, "Vector2.up() X")
assert_equal(up:getY(), 1.0, "Vector2.up() Y")

-- 测试GameObject类
print("\n--- Testing GameObject Class ---")

local obj = engine.core.GameObject.new()
assert_true(obj:getId() > 0, "GameObject auto-generated ID")
assert_true(string.find(obj:getName(), "GameObject_") == 1, "GameObject auto-generated name")
assert_equal(obj:getType(), engine.core.ObjectType.UNKNOWN, "GameObject default type")
assert_true(obj:isActive(), "GameObject default active state")

local obj2 = engine.core.GameObject.new("TestObject", engine.core.ObjectType.ITEM)
assert_equal(obj2:getName(), "TestObject", "GameObject custom name")
assert_equal(obj2:getType(), engine.core.ObjectType.ITEM, "GameObject custom type")

-- 测试属性设置
obj:setName("MyObject")
assert_equal(obj:getName(), "MyObject", "GameObject setName")

obj:setType(engine.core.ObjectType.PLAYER)
assert_equal(obj:getType(), engine.core.ObjectType.PLAYER, "GameObject setType")

obj:setActive(false)
assert_false(obj:isActive(), "GameObject setActive false")

-- 测试变换属性
local pos = engine.core.Vector2.new(10, 20)
obj:setPosition(pos)
local retrieved_pos = obj:getPosition()
assert_equal(retrieved_pos:getX(), 10.0, "GameObject position X")
assert_equal(retrieved_pos:getY(), 20.0, "GameObject position Y")

obj:setRotation(45.0)
assert_equal(obj:getRotation(), 45.0, "GameObject rotation")

local scale = engine.core.Vector2.new(2, 3)
obj:setScale(scale)
local retrieved_scale = obj:getScale()
assert_equal(retrieved_scale:getX(), 2.0, "GameObject scale X")
assert_equal(retrieved_scale:getY(), 3.0, "GameObject scale Y")

-- 测试标签系统
obj:addTag("player")
obj:addTag("important")
assert_true(obj:hasTag("player"), "GameObject hasTag positive")
assert_false(obj:hasTag("enemy"), "GameObject hasTag negative")

obj:removeTag("player")
assert_false(obj:hasTag("player"), "GameObject removeTag")

-- 测试静态方法
local next_id = engine.core.GameObject.getNextId()
assert_true(next_id > 0, "GameObject.getNextId")

local created_obj = engine.core.GameObject.create("CreatedObject", engine.core.ObjectType.ENEMY)
assert_equal(created_obj:getName(), "CreatedObject", "GameObject.create name")
assert_equal(created_obj:getType(), engine.core.ObjectType.ENEMY, "GameObject.create type")

-- 测试Character类
print("\n--- Testing Character Class ---")

local char = engine.core.Character.new()
assert_equal(char:getHealth(), 100.0, "Character default health")
assert_equal(char:getMaxHealth(), 100.0, "Character default max health")
assert_equal(char:getSpeed(), 100.0, "Character default speed")
assert_true(char:isAlive(), "Character default alive state")

local char2 = engine.core.Character.new("Hero", 150.0, 120.0)
assert_equal(char2:getName(), "Hero", "Character custom name")
assert_equal(char2:getHealth(), 150.0, "Character custom health")
assert_equal(char2:getSpeed(), 120.0, "Character custom speed")

-- 测试健康系统
char:setHealth(50.0)
assert_equal(char:getHealth(), 50.0, "Character setHealth")

char:takeDamage(20.0)
assert_equal(char:getHealth(), 30.0, "Character takeDamage")

char:heal(15.0)
assert_equal(char:getHealth(), 45.0, "Character heal")

char:setMaxHealth(200.0)
assert_equal(char:getMaxHealth(), 200.0, "Character setMaxHealth")

-- 测试移动系统
local move_dir = engine.core.Vector2.new(1, 0)
char:move(move_dir)
assert_true(char:isMoving(), "Character isMoving after move")

char:stop()
assert_false(char:isMoving(), "Character isMoving after stop")

local target = engine.core.Vector2.new(100, 100)
char:moveTo(target)
assert_true(char:isMoving(), "Character isMoving after moveTo")

-- 测试Player类
print("\n--- Testing Player Class ---")

local player = engine.core.Player.new()
assert_equal(player:getLevel(), 1, "Player default level")
assert_equal(player:getExperience(), 0, "Player default experience")
assert_equal(player:getScore(), 0, "Player default score")
assert_equal(player:getLives(), 3, "Player default lives")

local player2 = engine.core.Player.new("MainPlayer")
assert_equal(player2:getName(), "MainPlayer", "Player custom name")

-- 测试经验和升级
player:addExperience(50)
assert_equal(player:getExperience(), 50, "Player addExperience")

player:addExperience(60)  -- 总共110经验，应该升级到2级，剩余10经验
assert_equal(player:getLevel(), 2, "Player level up")
assert_equal(player:getExperience(), 10, "Player experience after level up")

-- 测试分数和生命
player:addScore(1000)
assert_equal(player:getScore(), 1000, "Player addScore")

player:setLives(5)
assert_equal(player:getLives(), 5, "Player setLives")

-- 测试能力系统
player:addAbility("Fireball")
player:addAbility("Heal")
assert_true(player:hasAbility("Fireball"), "Player hasAbility positive")
assert_false(player:hasAbility("Lightning"), "Player hasAbility negative")

-- 测试输入处理
player:handleInput("move_up", true)
player:handleInput("move_right", true)
assert_true(player:isMoving(), "Player moving after input")

-- 测试Enemy类
print("\n--- Testing Enemy Class ---")

local enemy = engine.core.Enemy.new()
assert_equal(enemy:getName(), "Enemy", "Enemy default name")
assert_equal(enemy:getHealth(), 50.0, "Enemy default health")

local enemy2 = engine.core.Enemy.new("Orc", 80.0, 15.0)
assert_equal(enemy2:getName(), "Orc", "Enemy custom name")
assert_equal(enemy2:getHealth(), 80.0, "Enemy custom health")
assert_equal(enemy2:getDamage(), 15.0, "Enemy custom damage")

-- 测试敌人属性
enemy:setDamage(12.0)
assert_equal(enemy:getDamage(), 12.0, "Enemy setDamage")

enemy:setAttackRange(50.0)
assert_equal(enemy:getAttackRange(), 50.0, "Enemy setAttackRange")

enemy:setDetectionRange(100.0)
assert_equal(enemy:getDetectionRange(), 100.0, "Enemy setDetectionRange")

-- 测试AI目标
enemy:setTarget(player)
local target = enemy:getTarget()
assert_equal(target:getName(), player:getName(), "Enemy setTarget/getTarget")

-- 测试MathUtils静态类
print("\n--- Testing MathUtils Static Class ---")

-- 测试基本数学函数
assert_equal(engine.utils.MathUtils.clamp(5.0, 0.0, 10.0), 5.0, "MathUtils.clamp normal value")
assert_equal(engine.utils.MathUtils.clamp(-5.0, 0.0, 10.0), 0.0, "MathUtils.clamp below min")
assert_equal(engine.utils.MathUtils.clamp(15.0, 0.0, 10.0), 10.0, "MathUtils.clamp above max")

assert_equal(engine.utils.MathUtils.lerp(0.0, 10.0, 0.5), 5.0, "MathUtils.lerp")
assert_equal(engine.utils.MathUtils.lerp(0.0, 10.0, 0.0), 0.0, "MathUtils.lerp at start")
assert_equal(engine.utils.MathUtils.lerp(0.0, 10.0, 1.0), 10.0, "MathUtils.lerp at end")

-- 测试随机数生成
local rand_int = engine.utils.MathUtils.randomInt(1, 10)
assert_true(rand_int >= 1 and rand_int <= 10, "MathUtils.randomInt in range")

local rand_float = engine.utils.MathUtils.randomFloat(0.0, 1.0)
assert_true(rand_float >= 0.0 and rand_float <= 1.0, "MathUtils.randomFloat in range")

local rand_bool = engine.utils.MathUtils.randomBool()
assert_true(rand_bool == true or rand_bool == false, "MathUtils.randomBool returns boolean")

-- 测试角度转换
assert_near(engine.utils.MathUtils.degreesToRadians(180.0), 3.141592653589793, 1e-10, "MathUtils.degreesToRadians")
assert_near(engine.utils.MathUtils.radiansToDegrees(3.141592653589793), 180.0, 1e-10, "MathUtils.radiansToDegrees")

-- 测试几何函数
local vec_a = engine.core.Vector2.new(0, 0)
local vec_b = engine.core.Vector2.new(3, 4)
assert_equal(engine.utils.MathUtils.distance(vec_a, vec_b), 5.0, "MathUtils.distance")

-- 测试常量
assert_near(engine.utils.MathUtils.PI, 3.141592653589793, 1e-10, "MathUtils.PI constant")
assert_near(engine.utils.MathUtils.TWO_PI, 6.283185307179586, 1e-10, "MathUtils.TWO_PI constant")

-- 测试CollisionUtils静态类
print("\n--- Testing CollisionUtils Static Class ---")

local point = engine.core.Vector2.new(5, 5)
local circle_center = engine.core.Vector2.new(0, 0)
assert_true(engine.utils.CollisionUtils.pointInCircle(point, circle_center, 10.0), "CollisionUtils.pointInCircle positive")
assert_false(engine.utils.CollisionUtils.pointInCircle(point, circle_center, 3.0), "CollisionUtils.pointInCircle negative")

local rect_pos = engine.core.Vector2.new(0, 0)
local rect_size = engine.core.Vector2.new(10, 10)
local test_point = engine.core.Vector2.new(5, 5)
assert_true(engine.utils.CollisionUtils.pointInRect(test_point, rect_pos, rect_size), "CollisionUtils.pointInRect positive")

local outside_point = engine.core.Vector2.new(15, 15)
assert_false(engine.utils.CollisionUtils.pointInRect(outside_point, rect_pos, rect_size), "CollisionUtils.pointInRect negative")

-- 测试形状碰撞
local pos1 = engine.core.Vector2.new(0, 0)
local pos2 = engine.core.Vector2.new(5, 0)
assert_true(engine.utils.CollisionUtils.circleToCircle(pos1, 3.0, pos2, 3.0), "CollisionUtils.circleToCircle positive")
assert_false(engine.utils.CollisionUtils.circleToCircle(pos1, 2.0, pos2, 2.0), "CollisionUtils.circleToCircle negative")

-- 测试ResourceManager类
print("\n--- Testing ResourceManager Class ---")

local resource_mgr = engine.utils.ResourceManager.new()
assert_equal(resource_mgr:getResourceCount(), 0, "ResourceManager initial count")

-- 测试文本资源
assert_true(resource_mgr:loadText("welcome", "Welcome to the game!"), "ResourceManager loadText")
assert_equal(resource_mgr:getText("welcome"), "Welcome to the game!", "ResourceManager getText")
assert_true(resource_mgr:hasText("welcome"), "ResourceManager hasText positive")
assert_false(resource_mgr:hasText("nonexistent"), "ResourceManager hasText negative")

-- 测试配置资源
local config = {
    health = "100",
    speed = "50",
    difficulty = "normal"
}
assert_true(resource_mgr:loadConfig("game_config", config), "ResourceManager loadConfig")
assert_equal(resource_mgr:getConfigValue("game_config", "health"), "100", "ResourceManager getConfigValue")
assert_equal(resource_mgr:getConfigValue("game_config", "nonexistent"), "", "ResourceManager getConfigValue missing key")

-- 测试预制体
local prefab = engine.core.GameObject.new("EnemyTemplate", engine.core.ObjectType.ENEMY)
prefab:addTag("enemy")
assert_true(resource_mgr:loadPrefab("basic_enemy", prefab), "ResourceManager loadPrefab")

local created = resource_mgr:createFromPrefab("basic_enemy")
assert_true(created ~= nil, "ResourceManager createFromPrefab returns object")
if created then
    assert_equal(created:getName(), "EnemyTemplate", "ResourceManager prefab copy name")
    assert_equal(created:getType(), engine.core.ObjectType.ENEMY, "ResourceManager prefab copy type")
    assert_true(created:hasTag("enemy"), "ResourceManager prefab copy tags")
end

assert_true(resource_mgr:getResourceCount() > 0, "ResourceManager count after loading")

-- 测试单例类（World和GameManager）
print("\n--- Testing Singleton Classes ---")

-- 测试World单例
local world = engine.systems.World.getInstance()
assert_true(world ~= nil, "World.getInstance returns object")

-- 创建另一个引用，应该是同一个实例
local world2 = engine.systems.World.getInstance()
-- 注意：在Lua中，即使是同一个C++对象，也可能有不同的userdata包装
-- 所以我们通过行为来测试单例特性

world:clear()
assert_equal(world:getObjectCount(), 0, "World initial object count")

local test_obj = engine.core.GameObject.new("TestObj", engine.core.ObjectType.ITEM)
world:addGameObject(test_obj)
assert_equal(world:getObjectCount(), 1, "World object count after adding")

-- 通过world2访问，应该看到同样的对象
assert_equal(world2:getObjectCount(), 1, "World singleton behavior - same count through different reference")

-- 测试GameManager单例
local game_mgr = engine.systems.GameManager.getInstance()
assert_true(game_mgr ~= nil, "GameManager.getInstance returns object")

assert_equal(game_mgr:getGameState(), engine.core.GameState.MENU, "GameManager initial state")
assert_false(game_mgr:isGameRunning(), "GameManager initial not running")

game_mgr:setGameState(engine.core.GameState.PLAYING)
assert_equal(game_mgr:getGameState(), engine.core.GameState.PLAYING, "GameManager setGameState")
assert_true(game_mgr:isGameRunning(), "GameManager isGameRunning after setting PLAYING")

-- 测试EventManager类
print("\n--- Testing EventManager Class ---")

local event_mgr = engine.systems.EventManager.new()
assert_true(event_mgr ~= nil, "EventManager creation")

-- 设置事件回调
local event_triggered = false
event_mgr.OnGameStart = function()
    event_triggered = true
end

-- 触发事件
event_mgr:triggerGameStart()
assert_true(event_triggered, "EventManager OnGameStart callback triggered")

-- 测试带参数的事件
local player_name = ""
event_mgr.OnPlayerSpawn = function(player)
    player_name = player:getName()
end

local test_player = engine.core.Player.new("TestHero")
event_mgr:triggerPlayerSpawn(test_player)
assert_equal(player_name, "TestHero", "EventManager OnPlayerSpawn callback with parameter")

print("\n=== Test Summary ===")
print("Tests passed: " .. tests_passed)
print("Tests failed: " .. tests_failed)
print("Total tests: " .. (tests_passed + tests_failed))

if tests_failed == 0 then
    print("🎉 All tests passed!")
    return 0
else
    print("❌ Some tests failed!")
    return 1
end