/**
 * @file game_engine_main.cpp
 * @brief 游戏引擎示例的测试主程序
 */

#include "game_engine.h"
#include <sol/sol.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>

// Forward declaration of binding function
void register_generated_module_bindings(sol::state& lua);

using namespace engine;

void runGameEngineDemo() {
    std::cout << "=== Game Engine Demo ===" << std::endl;
    
    // 获取游戏管理器和世界实例
    auto& gameManager = systems::GameManager::getInstance();
    auto& world = systems::World::getInstance();
    
    // 初始化游戏
    gameManager.initializeGame();
    
    // 创建事件管理器
    auto eventManager = std::make_unique<systems::EventManager>();
    
    // 设置事件回调
    eventManager->OnGameStart = []() {
        std::cout << "Game started event triggered!" << std::endl;
    };
    
    eventManager->OnPlayerSpawn = [](std::shared_ptr<core::Player> player) {
        std::cout << "Player spawned: " << player->getName() << std::endl;
    };
    
    eventManager->OnPlayerLevelUp = [](std::shared_ptr<core::Player> player, int newLevel) {
        std::cout << "Player " << player->getName() << " reached level " << newLevel << "!" << std::endl;
    };
    
    eventManager->OnEnemyDeath = [](std::shared_ptr<core::Enemy> enemy) {
        std::cout << "Enemy defeated: " << enemy->getName() << std::endl;
    };
    
    std::cout << "\n--- Creating Game Objects ---" << std::endl;
    
    // 创建玩家
    auto player = std::make_shared<core::Player>("Hero");
    player->setPosition(core::Vector2(100, 100));
    player->addAbility("Fireball");
    player->addAbility("Heal");
    world.addGameObject(player);
    eventManager->triggerPlayerSpawn(player);
    
    // 创建一些敌人
    for (int i = 0; i < 3; ++i) {
        auto enemy = std::make_shared<core::Enemy>("Goblin_" + std::to_string(i + 1), 30.0, 8.0);
        enemy->setPosition(core::Vector2(200 + i * 50, 150 + i * 30));
        enemy->setTarget(player);
        
        // 设置巡逻路径
        std::vector<core::Vector2> patrolPoints = {
            core::Vector2(200 + i * 50, 100),
            core::Vector2(300 + i * 50, 100),
            core::Vector2(300 + i * 50, 200),
            core::Vector2(200 + i * 50, 200)
        };
        enemy->patrol(patrolPoints);
        
        world.addGameObject(enemy);
        eventManager->triggerEnemySpawn(enemy);
    }
    
    // 创建一些环境对象
    auto treasure = std::make_shared<core::GameObject>("Treasure Chest", core::ObjectType::ITEM);
    treasure->setPosition(core::Vector2(400, 300));
    treasure->addTag("collectible");
    treasure->addTag("valuable");
    world.addGameObject(treasure);
    
    auto barrier = std::make_shared<core::GameObject>("Stone Wall", core::ObjectType::ENVIRONMENT);
    barrier->setPosition(core::Vector2(250, 250));
    barrier->setScale(core::Vector2(20, 100)); // 宽20，高100
    barrier->addTag("solid");
    world.addGameObject(barrier);
    
    std::cout << "\n--- Testing Math Utilities ---" << std::endl;
    
    // 测试数学工具
    core::Vector2 vec1(3, 4);
    core::Vector2 vec2(1, 2);
    
    std::cout << "Vector1: (" << vec1.getX() << ", " << vec1.getY() << ")" << std::endl;
    std::cout << "Vector2: (" << vec2.getX() << ", " << vec2.getY() << ")" << std::endl;
    std::cout << "Vector1 length: " << vec1.length() << std::endl;
    std::cout << "Distance between vectors: " << vec1.distance(vec2) << std::endl;
    
    auto vec3 = vec1 + vec2;
    std::cout << "Vector1 + Vector2: (" << vec3.getX() << ", " << vec3.getY() << ")" << std::endl;
    
    auto vec4 = vec1 * 2.0;
    std::cout << "Vector1 * 2: (" << vec4.getX() << ", " << vec4.getY() << ")" << std::endl;
    
    // 测试随机数生成
    std::cout << "Random int (1-10): " << utils::MathUtils::randomInt(1, 10) << std::endl;
    std::cout << "Random float (0-1): " << utils::MathUtils::randomFloat(0.0, 1.0) << std::endl;
    std::cout << "Random bool: " << (utils::MathUtils::randomBool() ? "true" : "false") << std::endl;
    
    auto randomVec = utils::MathUtils::randomVector2(-10, 10);
    std::cout << "Random Vector2: (" << randomVec.getX() << ", " << randomVec.getY() << ")" << std::endl;
    
    // 测试角度转换
    double degrees = 90.0;
    double radians = utils::MathUtils::degreesToRadians(degrees);
    std::cout << degrees << " degrees = " << radians << " radians" << std::endl;
    std::cout << radians << " radians = " << utils::MathUtils::radiansToDegrees(radians) << " degrees" << std::endl;
    
    std::cout << "\n--- Testing Collision Detection ---" << std::endl;
    
    // 测试碰撞检测
    core::Vector2 point(150, 150);
    core::Vector2 circleCenter(100, 100);
    double circleRadius = 50.0;
    
    bool pointInCircle = utils::CollisionUtils::pointInCircle(point, circleCenter, circleRadius);
    std::cout << "Point (" << point.getX() << ", " << point.getY() << ") in circle: " 
              << (pointInCircle ? "true" : "false") << std::endl;
    
    // 测试游戏对象之间的碰撞
    auto allObjects = world.getAllGameObjects();
    std::cout << "Checking collisions for " << allObjects.size() << " objects..." << std::endl;
    
    for (const auto& obj : allObjects) {
        auto collisions = utils::CollisionUtils::findCollisions(obj, allObjects);
        if (!collisions.empty()) {
            std::cout << obj->getName() << " collides with " << collisions.size() << " objects" << std::endl;
        }
    }
    
    std::cout << "\n--- Testing Resource Manager ---" << std::endl;
    
    // 创建资源管理器
    auto resourceManager = std::make_unique<utils::ResourceManager>();
    
    // 加载文本资源
    resourceManager->loadText("welcome_message", "Welcome to the game!");
    resourceManager->loadText("game_over_message", "Game Over - Thanks for playing!");
    
    // 加载配置
    std::unordered_map<std::string, std::string> gameConfig = {
        {"max_health", "100"},
        {"start_level", "1"},
        {"difficulty", "normal"}
    };
    resourceManager->loadConfig("game_settings", gameConfig);
    
    // 加载预制体
    auto enemyPrefab = std::make_shared<core::GameObject>("Enemy Template", core::ObjectType::ENEMY);
    enemyPrefab->setScale(core::Vector2(16, 16));
    enemyPrefab->addTag("enemy");
    enemyPrefab->addTag("ai_controlled");
    resourceManager->loadPrefab("basic_enemy", enemyPrefab);
    
    // 测试资源访问
    std::cout << "Welcome message: " << resourceManager->getText("welcome_message") << std::endl;
    std::cout << "Max health setting: " << resourceManager->getConfigValue("game_settings", "max_health") << std::endl;
    std::cout << "Loaded resources count: " << resourceManager->getResourceCount() << std::endl;
    
    // 从预制体创建对象
    auto newEnemy = resourceManager->createFromPrefab("basic_enemy");
    if (newEnemy) {
        std::cout << "Created enemy from prefab: " << newEnemy->getName() << std::endl;
        std::cout << "Enemy tags: ";
        for (const auto& tag : newEnemy->getTags()) {
            std::cout << tag << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "\n--- Starting Game Simulation ---" << std::endl;
    
    // 启动游戏
    gameManager.startGame();
    eventManager->triggerGameStart();
    
    // 模拟游戏循环（运行几秒钟）
    int frames = 0;
    const int maxFrames = 180; // 约3秒钟（假设60FPS）
    
    std::cout << "Running game simulation for " << maxFrames << " frames..." << std::endl;
    
    while (frames < maxFrames && gameManager.isGameRunning()) {
        gameManager.update();
        
        // 模拟玩家输入（简单的移动）
        if (frames % 60 == 0) { // 每秒一次
            // 让玩家向随机方向移动
            auto randomDir = utils::MathUtils::randomVector2(-1, 1).normalized();
            player->move(randomDir);
            
            // 给玩家一些经验值
            player->addExperience(25);
            
            // 给玩家一些分数
            player->addScore(100);
            
            std::cout << "Frame " << frames << " - Player at (" 
                      << player->getPosition().getX() << ", " << player->getPosition().getY() 
                      << "), Level: " << player->getLevel() 
                      << ", Score: " << player->getScore() << std::endl;
        }
        
        // 检查玩家是否升级
        static int lastLevel = player->getLevel();
        if (player->getLevel() > lastLevel) {
            eventManager->triggerPlayerLevelUp(player, player->getLevel());
            lastLevel = player->getLevel();
        }
        
        // 模拟帧率
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        frames++;
    }
    
    std::cout << "\n--- Game Simulation Complete ---" << std::endl;
    std::cout << "World object count: " << world.getObjectCount() << std::endl;
    std::cout << "Game time: " << gameManager.getGameTime() << " seconds" << std::endl;
    std::cout << "Player final level: " << player->getLevel() << std::endl;
    std::cout << "Player final score: " << player->getScore() << std::endl;
    std::cout << "Player abilities: ";
    for (const auto& ability : player->getAbilities()) {
        std::cout << ability << " ";
    }
    std::cout << std::endl;
    
    // 查找特定类型的对象
    auto enemies = world.findGameObjectsByType(core::ObjectType::ENEMY);
    std::cout << "Enemies in world: " << enemies.size() << std::endl;
    
    auto collectibles = world.findGameObjectsByTag("collectible");
    std::cout << "Collectible objects: " << collectibles.size() << std::endl;
    
    // 停止游戏
    gameManager.stopGame();
    eventManager->triggerGameEnd();
    
    // 清理
    world.clear();
    
    std::cout << "\n=== Game Engine Demo Complete ===" << std::endl;
}

void runLuaTests() {
    std::cout << "\n=== Running Lua Tests ===" << std::endl;
    
    // Initialize Lua state
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, 
                      sol::lib::math, sol::lib::table, sol::lib::io);
    
    try {
        // Register all C++ bindings
        register_generated_module_bindings(lua);
        
        // Look for Lua test scripts in the scripts directory
        std::string scripts_dir = "examples/scripts";
        if (!std::filesystem::exists(scripts_dir)) {
            scripts_dir = "scripts"; // Try relative path
        }
        
        if (std::filesystem::exists(scripts_dir)) {
            std::cout << "Found scripts directory: " << scripts_dir << std::endl;
            
            // List of test scripts to run
            std::vector<std::string> test_scripts = {
                "test_simple.lua",
                "test_game_engine.lua",
                "test_comprehensive.lua"
            };
            
            for (const auto& script : test_scripts) {
                std::string script_path = scripts_dir + "/" + script;
                if (std::filesystem::exists(script_path)) {
                    std::cout << "\n--- Running " << script << " ---" << std::endl;
                    try {
                        lua.script_file(script_path);
                    } catch (const sol::error& e) {
                        std::cerr << "Lua error in " << script << ": " << e.what() << std::endl;
                    }
                } else {
                    std::cout << "Script not found: " << script_path << std::endl;
                }
            }
        } else {
            std::cout << "Scripts directory not found. Creating simple test..." << std::endl;
            
            // Run a simple inline test
            lua.script(R"lua(
                print("=== Inline Lua Test ===")
                
                -- Test simple module
                if simple then
                    print("Simple module found")
                    if simple.add then
                        local result = simple.add(2, 3)
                        print("simple.add(2, 3) = " .. result)
                        assert(result == 5, "Addition test failed")
                    end
                else
                    print("Simple module not found")
                end
                
                -- Test engine module  
                if engine and engine.core then
                    print("Engine.core module found")
                    
                    -- Test Vector2
                    if engine.core.Vector2 then
                        local v1 = engine.core.Vector2(3, 4)
                        print("Created Vector2(3, 4)")
                        print("Vector length: " .. v1:length())
                    end
                else
                    print("Engine.core module not found")
                end
                
                print("Inline test completed")
            )lua");
        }
        
    } catch (const sol::error& e) {
        std::cerr << "Lua initialization error: " << e.what() << std::endl;
    }
    
    std::cout << "\n=== Lua Tests Complete ===" << std::endl;
}

int main() {
    try {
        runGameEngineDemo();
        runLuaTests();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}