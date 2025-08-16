/**
 * comprehensive_main.cpp
 * 综合测试示例的主程序
 * 
 * 这个文件演示了comprehensive_test.h中所有复杂特性的实际使用
 */

#include "comprehensive_test.h"
#include <sol/sol.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>

// Forward declaration of binding function
void register_generated_module_bindings(sol::state& lua);

void runComprehensiveDemo() {
    std::cout << "=== Comprehensive Demo ===" << std::endl;
    std::cout << "Demonstrating advanced C++ features for Lua binding generation" << std::endl;
    std::cout << std::endl;

    try {
        // 测试枚举
        std::cout << "--- Testing Enums ---" << std::endl;
        std::cout << "Status::ACTIVE = " << static_cast<int>(game::core::Status::ACTIVE) << std::endl;
        std::cout << "Priority::HIGH = " << static_cast<int>(game::core::Priority::HIGH) << std::endl;
        std::cout << std::endl;

        // 测试常量
        std::cout << "--- Testing Constants ---" << std::endl;
        std::cout << "MAX_PLAYERS = " << game::core::MAX_PLAYERS << std::endl;
        std::cout << "PI = " << game::core::PI << std::endl;
        std::cout << "GAME_NAME = \"" << game::core::GAME_NAME << "\"" << std::endl;
        std::cout << std::endl;

        // 测试全局函数
        std::cout << "--- Testing Global Functions ---" << std::endl;
        double distance = game::core::calculateDistance(0, 0, 3, 4);
        std::cout << "Distance from (0,0) to (3,4) = " << distance << std::endl;
        
        std::string message = game::core::formatMessage("Test message", 1);
        std::cout << "Formatted message: " << message << std::endl;
        
        bool valid = game::core::validateInput("test input");
        std::cout << "Input validation result: " << (valid ? "valid" : "invalid") << std::endl;
        std::cout << std::endl;

        // 测试Entity基类
        std::cout << "--- Testing Entity Base Class ---" << std::endl;
        game::core::Entity entity1;
        std::cout << "Entity1 - ID: " << entity1.getId() << ", Name: " << entity1.getName() << std::endl;
        
        game::core::Entity entity2(42, "CustomEntity");
        std::cout << "Entity2 - ID: " << entity2.getId() << ", Name: " << entity2.getName() << std::endl;
        
        // 测试静态方法
        int nextId = game::core::Entity::getNextId();
        std::cout << "Next ID: " << nextId << std::endl;
        
        game::core::Entity::resetIdCounter();
        game::core::Entity new_entity;
        std::cout << "Entity ID after reset: " << new_entity.getId() << std::endl;
        std::cout << std::endl;

        // 测试Player类（继承）
        std::cout << "--- Testing Player Class (Inheritance) ---" << std::endl;
        game::core::Player player1;
        std::cout << "Player1 - Level: " << player1.getLevel() << ", Health: " << player1.getHealth() << std::endl;
        
        game::core::Player player2(100, "Hero", 5);
        std::cout << "Player2 - ID: " << player2.getId() << ", Name: " << player2.getName() 
                  << ", Level: " << player2.getLevel() << std::endl;
        
        // 测试技能和物品系统
        player2.addSkill("Fireball");
        player2.addSkill("Lightning Bolt");
        player2.addItem("Health Potion", 3);
        player2.addItem("Mana Potion", 2);
        
        auto skills = player2.getSkills();
        std::cout << "Player2 skills: ";
        for (const auto& skill : skills) {
            std::cout << skill << " ";
        }
        std::cout << std::endl;
        
        // 测试多态
        std::cout << "Entity toString: " << entity2.toString() << std::endl;
        std::cout << "Player toString: " << player2.toString() << std::endl;
        
        // 测试Player特有属性
        player2.setLevel(3);
        std::cout << "Player level after set: " << player2.getLevel() << std::endl;
        
        player2.setHealth(75.0);
        std::cout << "Player health after set: " << player2.getHealth() << std::endl;
        
        player2.setMana(60.0);
        std::cout << "Player mana after set: " << player2.getMana() << std::endl;
        std::cout << std::endl;

        // 测试GameManager单例
        std::cout << "--- Testing GameManager Singleton ---" << std::endl;
        game::core::GameManager& gameManager = game::core::GameManager::getInstance();
        
        std::cout << "Game running: " << (gameManager.isGameRunning() ? "Yes" : "No") << std::endl;
        
        gameManager.startGame();
        std::cout << "After startGame() - Running: " << (gameManager.isGameRunning() ? "Yes" : "No") << std::endl;
        
        gameManager.addPlayer(std::make_shared<game::core::Player>(player2));
        std::cout << "Player count: " << gameManager.getPlayerCount() << std::endl;
        
        auto retrieved_player = gameManager.getPlayer(player2.getId());
        if (retrieved_player) {
            std::cout << "Retrieved player: " << retrieved_player->getName() << std::endl;
        }
        
        auto all_players = gameManager.getAllPlayers();
        std::cout << "All players count: " << all_players.size() << std::endl;
        
        gameManager.stopGame();
        std::cout << "After stopGame() - Running: " << (gameManager.isGameRunning() ? "Yes" : "No") << std::endl;
        std::cout << std::endl;

        // 测试MathUtils静态类
        std::cout << "--- Testing MathUtils Static Class ---" << std::endl;
        std::cout << "Clamp(15, 0, 10) = " << game::core::MathUtils::clamp(15.0, 0.0, 10.0) << std::endl;
        std::cout << "Lerp(0, 10, 0.5) = " << game::core::MathUtils::lerp(0.0, 10.0, 0.5) << std::endl;
        std::cout << "Random(1, 100) = " << game::core::MathUtils::random(1, 100) << std::endl;
        std::cout << "Dot product (1,0)·(0,1) = " << game::core::MathUtils::dotProduct(1, 0, 0, 1) << std::endl;
        std::cout << "Magnitude (3,4) = " << game::core::MathUtils::magnitude(3, 4) << std::endl;
        std::cout << "PI = " << game::core::MathUtils::PI << std::endl;
        std::cout << "E = " << game::core::MathUtils::E << std::endl;
        std::cout << std::endl;

        // 测试TransformComponent
        std::cout << "--- Testing TransformComponent ---" << std::endl;
        game::core::TransformComponent transform(10.0, 20.0, 45.0);
        std::cout << "Transform - X: " << transform.getX() << ", Y: " << transform.getY() 
                  << ", Rotation: " << transform.getRotation() << std::endl;
        
        transform.translate(5.0, 10.0);
        std::cout << "After translate(5, 10) - X: " << transform.getX() << ", Y: " << transform.getY() << std::endl;
        
        transform.rotate(45.0);
        std::cout << "After rotate(45) - Rotation: " << transform.getRotation() << std::endl;
        
        std::cout << "Component type: " << transform.getTypeName() << std::endl;
        std::cout << "Is active: " << (transform.isActive() ? "Yes" : "No") << std::endl;
        
        transform.setActive(false);
        std::cout << "After setActive(false): " << (transform.isActive() ? "Yes" : "No") << std::endl;
        std::cout << std::endl;

        // 测试EventSystem回调
        std::cout << "--- Testing EventSystem Callbacks ---" << std::endl;
        game::events::EventSystem eventSystem;
        
        // 设置回调
        eventSystem.OnGameStart = []() {
            std::cout << "Game started callback triggered!" << std::endl;
        };
        
        eventSystem.OnPlayerJoin = [](std::shared_ptr<game::core::Player> player) {
            std::cout << "Player joined: " << player->getName() << std::endl;
        };
        
        eventSystem.OnPlayerLevelUp = [](std::shared_ptr<game::core::Player> player, int oldLevel, int newLevel) {
            std::cout << "Player " << player->getName() << " leveled up from " 
                      << oldLevel << " to " << newLevel << std::endl;
        };
        
        eventSystem.OnValidateAction = [](const std::string& action, double value) -> bool {
            std::cout << "Validating action: " << action << " with value: " << value << std::endl;
            return value > 0.0;
        };
        
        // 触发事件
        eventSystem.triggerGameStart();
        eventSystem.triggerPlayerJoin(std::make_shared<game::core::Player>(player2));
        eventSystem.triggerPlayerLevelUp(std::make_shared<game::core::Player>(player2), 5, 6);
        
        bool validateResult = eventSystem.validateAction("jump", 10.0);
        std::cout << "Validate action result: " << (validateResult ? "valid" : "invalid") << std::endl;
        std::cout << std::endl;

        // 测试容器绑定
        std::cout << "--- Testing STL Container Bindings ---" << std::endl;
        game::containers::ContainerUtils containerUtils;
        
        auto intVec = containerUtils.getIntVector();
        std::cout << "Int vector size: " << intVec.size() << std::endl;
        std::cout << "Int vector contents: ";
        for (int val : intVec) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
        
        auto stringVec = containerUtils.getStringVector();
        std::cout << "String vector size: " << stringVec.size() << std::endl;
        std::cout << "String vector contents: ";
        for (const auto& str : stringVec) {
            std::cout << "\"" << str << "\" ";
        }
        std::cout << std::endl;
        
        auto stringIntMap = containerUtils.getStringIntMap();
        std::cout << "String-Int map size: " << stringIntMap.size() << std::endl;
        std::cout << "Map contents: ";
        for (const auto& pair : stringIntMap) {
            std::cout << "[\"" << pair.first << "\": " << pair.second << "] ";
        }
        std::cout << std::endl;
        
        auto stringDoubleMap = containerUtils.getStringDoubleMap();
        std::cout << "String-Double map size: " << stringDoubleMap.size() << std::endl;
        std::cout << "Double map contents: ";
        for (const auto& pair : stringDoubleMap) {
            std::cout << "[\"" << pair.first << "\": " << pair.second << "] ";
        }
        std::cout << std::endl;
        
        // 测试容器处理方法
        std::vector<int> testVec = {1, 2, 3, 4, 5};
        containerUtils.processIntVector(testVec);
        
        std::vector<std::string> testStrVec = {"hello", "world", "lua"};
        containerUtils.processStringVector(testStrVec);
        std::cout << std::endl;

        // 测试智能指针
        std::cout << "--- Testing Smart Pointer Demo ---" << std::endl;
        game::smartptr::SmartPointerDemo smartDemo;
        
        auto newPlayer = smartDemo.createPlayer("SmartPlayer");
        std::cout << "Created player: " << newPlayer->getName() << std::endl;
        
        auto newEntity = smartDemo.createEntity(999, "SmartEntity");
        std::cout << "Created entity: ID=" << newEntity->getId() << ", Name=" << newEntity->getName() << std::endl;
        
        smartDemo.setCurrentPlayer(newPlayer);
        auto currentPlayer = smartDemo.getCurrentPlayer();
        if (currentPlayer) {
            std::cout << "Current player: " << currentPlayer->getName() << std::endl;
        }
        
        // 测试weak_ptr
        auto weakPlayer = smartDemo.getPlayerRef(newPlayer->getId());
        if (auto lockedPlayer = weakPlayer.lock()) {
            std::cout << "Weak pointer locked to: " << lockedPlayer->getName() << std::endl;
        }
        
        bool playerValid = smartDemo.isPlayerValid(weakPlayer);
        std::cout << "Player weak reference valid: " << (playerValid ? "Yes" : "No") << std::endl;
        
        // 测试智能指针容器
        auto allPlayersFromDemo = smartDemo.getAllPlayers();
        std::cout << "All players from demo count: " << allPlayersFromDemo.size() << std::endl;
        
        // 测试Transform创建
        auto transform1 = smartDemo.createTransform();
        auto transform2 = smartDemo.createTransform(100.0, 200.0);
        std::cout << "Created transforms - Transform1 active: " << (transform1->isActive() ? "Yes" : "No") << std::endl;
        std::cout << "Transform2 position: (" << transform2->getX() << ", " << transform2->getY() << ")" << std::endl;
        std::cout << std::endl;

        // 测试Vector2D运算符重载
        std::cout << "--- Testing Vector2D Operator Overloading ---" << std::endl;
        operators::Vector2D vec1(3.0, 4.0);
        operators::Vector2D vec2(1.0, 2.0);
        
        std::cout << "vec1: (" << vec1.getX() << ", " << vec1.getY() << ")" << std::endl;
        std::cout << "vec2: (" << vec2.getX() << ", " << vec2.getY() << ")" << std::endl;
        
        // 测试数学运算符
        auto vec3 = vec1 + vec2;  // (3,4) + (1,2) = (4,6)
        std::cout << "vec1 + vec2 = (" << vec3.getX() << ", " << vec3.getY() << ")" << std::endl;
        
        auto vec4 = vec1 - vec2;  // (3,4) - (1,2) = (2,2)
        std::cout << "vec1 - vec2 = (" << vec4.getX() << ", " << vec4.getY() << ")" << std::endl;
        
        auto vec5 = vec1 * 2.0;  // (3,4) * 2 = (6,8)
        std::cout << "vec1 * 2 = (" << vec5.getX() << ", " << vec5.getY() << ")" << std::endl;
        
        auto vec6 = vec5 / 2.0;  // (6,8) / 2 = (3,4)
        std::cout << "vec5 / 2 = (" << vec6.getX() << ", " << vec6.getY() << ")" << std::endl;
        
        // 测试比较运算符
        bool equal = (vec1 == vec6);
        std::cout << "vec1 == vec6: " << (equal ? "true" : "false") << std::endl;
        
        bool notEqual = (vec1 != vec2);
        std::cout << "vec1 != vec2: " << (notEqual ? "true" : "false") << std::endl;
        
        // 测试一元运算符
        auto vec7 = -vec1;  // -(3,4) = (-3,-4)
        std::cout << "-vec1 = (" << vec7.getX() << ", " << vec7.getY() << ")" << std::endl;
        
        // 测试下标运算符
        std::cout << "vec1[0] = " << vec1[0] << ", vec1[1] = " << vec1[1] << std::endl;
        
        // 测试函数调用运算符（长度）
        std::cout << "vec1() (length) = " << vec1() << std::endl;
        
        // 测试工具方法
        std::cout << "vec1.length() = " << vec1.length() << std::endl;
        std::cout << "vec1.lengthSquared() = " << vec1.lengthSquared() << std::endl;
        std::cout << "vec1.dot(vec2) = " << vec1.dot(vec2) << std::endl;
        
        auto normalized = vec1.normalized();
        std::cout << "vec1 normalized: (" << normalized.getX() << ", " << normalized.getY() 
                  << ") length=" << normalized.length() << std::endl;
        std::cout << std::endl;

        // 测试容器工厂
        std::cout << "--- Testing STL Container Factory ---" << std::endl;
        factories::ContainerFactory containerFactory;
        
        // 测试基础 vector 创建
        auto emptyIntVec = factories::ContainerFactory::createIntVector();
        std::cout << "Created empty int vector, size: " << emptyIntVec.size() << std::endl;
        
        auto filledIntVec = factories::ContainerFactory::createIntVector(5, 42);
        std::cout << "Created filled int vector (5 elements, value 42): ";
        for (size_t i = 0; i < filledIntVec.size() && i < 5; ++i) {
            std::cout << filledIntVec[i] << " ";
        }
        std::cout << std::endl;
        
        auto rangeVec = factories::ContainerFactory::createRangeVector(1.0, 10.0, 2.0);
        std::cout << "Created range vector (1 to 10, step 2), size: " << rangeVec.size() << std::endl;
        std::cout << "Range values: ";
        for (size_t i = 0; i < rangeVec.size() && i < 10; ++i) {
            std::cout << rangeVec[i] << " ";
        }
        std::cout << std::endl;
        
        // 测试 map 创建
        auto emptyMap = factories::ContainerFactory::createStringIntMap();
        std::cout << "Created empty string-int map, size: " << emptyMap.size() << std::endl;
        
        auto prefilledMap = factories::ContainerFactory::createPrefilledStringIntMap();
        std::cout << "Created prefilled map, size: " << prefilledMap.size() << std::endl;
        std::cout << "Map contents: ";
        for (const auto& pair : prefilledMap) {
            std::cout << "[\"" << pair.first << "\": " << pair.second << "] ";
        }
        std::cout << std::endl;
        
        // 测试 map 键值提取
        auto keys = factories::ContainerFactory::extractKeysFromMap(prefilledMap);
        auto values = factories::ContainerFactory::extractValuesFromMap(prefilledMap);
        std::cout << "Extracted " << keys.size() << " keys and " << values.size() << " values" << std::endl;
        
        // 测试智能指针容器
        auto emptyPlayerVec = factories::ContainerFactory::createPlayerVector();
        std::cout << "Created empty player vector, size: " << emptyPlayerVec.size() << std::endl;
        
        auto playerVec = factories::ContainerFactory::createPlayerVector(3);
        std::cout << "Created player vector with 3 players: ";
        for (const auto& player : playerVec) {
            if (player) {
                std::cout << player->getName() << "(Lv." << player->getLevel() << ") ";
            }
        }
        std::cout << std::endl;
        
        // 测试嵌套容器
        auto matrix2D = factories::ContainerFactory::create2DIntVector(3, 4, 7);
        std::cout << "Created 3x4 2D matrix (filled with 7):" << std::endl;
        for (size_t i = 0; i < matrix2D.size(); ++i) {
            std::cout << "  Row " << i << ": [";
            for (size_t j = 0; j < matrix2D[i].size(); ++j) {
                std::cout << matrix2D[i][j];
                if (j < matrix2D[i].size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
        }
        
        auto mapOfVectors = factories::ContainerFactory::createMapOfVectors();
        std::cout << "Created map of vectors with " << mapOfVectors.size() << " sequences:" << std::endl;
        for (const auto& pair : mapOfVectors) {
            std::cout << "  " << pair.first << ": [";
            for (size_t i = 0; i < pair.second.size() && i < 8; ++i) {
                std::cout << pair.second[i];
                if (i < pair.second.size() - 1 && i < 7) std::cout << ", ";
            }
            if (pair.second.size() > 8) std::cout << "...";
            std::cout << "]" << std::endl;
        }
        
        // 测试容器操作
        auto doubleVec = factories::ContainerFactory::intVectorToDouble(filledIntVec);
        std::cout << "Converted int vector to double vector, size: " << doubleVec.size() << std::endl;
        
        auto emptyStrVec1 = factories::ContainerFactory::createStringVector();
        auto emptyStrVec2 = factories::ContainerFactory::createStringVector();
        auto mergedVec = factories::ContainerFactory::mergeStringVectors(emptyStrVec1, emptyStrVec2);
        std::cout << "Merged two empty string vectors, result size: " << mergedVec.size() << std::endl;
        
        // 测试统计分析
        auto statsTestVec = factories::ContainerFactory::createIntVector(10, 50);
        auto stats = factories::ContainerFactory::getVectorStats(statsTestVec);
        std::cout << "Vector stats - Size: " << stats.at("size") 
                  << ", Sum: " << stats.at("sum") 
                  << ", Avg: " << stats.at("avg") 
                  << ", Min: " << stats.at("min") 
                  << ", Max: " << stats.at("max") << std::endl;
        
        // 测试过滤和排序
        auto bigVec = factories::ContainerFactory::createIntVector(10, 75);
        auto filtered = factories::ContainerFactory::filterGreaterThan(bigVec, 70);
        std::cout << "Filtered vector (values > 70): original size " << bigVec.size() 
                  << ", filtered size " << filtered.size() << std::endl;
        
        auto unsortedVec = std::vector<int>{5, 2, 8, 1, 9, 3};
        auto sortedAsc = factories::ContainerFactory::sortVector(unsortedVec, true);
        auto sortedDesc = factories::ContainerFactory::sortVector(unsortedVec, false);
        std::cout << "Sorting test - Original: [5,2,8,1,9,3]" << std::endl;
        std::cout << "  Ascending: [";
        for (size_t i = 0; i < sortedAsc.size(); ++i) {
            std::cout << sortedAsc[i];
            if (i < sortedAsc.size() - 1) std::cout << ",";
        }
        std::cout << "]" << std::endl;
        std::cout << "  Descending: [";
        for (size_t i = 0; i < sortedDesc.size(); ++i) {
            std::cout << sortedDesc[i];
            if (i < sortedDesc.size() - 1) std::cout << ",";
        }
        std::cout << "]" << std::endl;
        
        std::cout << "Container factory tests completed successfully!" << std::endl;
        std::cout << std::endl;

        // 测试复杂场景
        std::cout << "--- Testing Complex Scenarios ---" << std::endl;
        
        // 复杂的对象交互
        auto mainPlayer = std::make_shared<game::core::Player>(1, "MainHero", 10);
        auto& worldMgr = game::core::GameManager::getInstance();
        
        worldMgr.startGame();
        worldMgr.addPlayer(mainPlayer);
        mainPlayer->addSkill("Lightning Bolt");
        mainPlayer->addSkill("Teleport");
        mainPlayer->addItem("Sword", 1);
        mainPlayer->addItem("Shield", 1);
        
        // 事件链测试
        game::events::EventSystem events;
        std::vector<std::string> eventLog;
        
        events.OnPlayerJoin = [&eventLog](std::shared_ptr<game::core::Player> player) {
            eventLog.push_back("Player " + player->getName() + " joined");
        };
        
        events.OnPlayerLevelUp = [&eventLog](std::shared_ptr<game::core::Player> player, int oldLvl, int newLvl) {
            eventLog.push_back("Player " + player->getName() + " leveled up from " + 
                              std::to_string(oldLvl) + " to " + std::to_string(newLvl));
        };
        
        events.triggerPlayerJoin(mainPlayer);
        events.triggerPlayerLevelUp(mainPlayer, 10, 11);
        
        std::cout << "Event log entries: " << eventLog.size() << std::endl;
        for (const auto& entry : eventLog) {
            std::cout << "  " << entry << std::endl;
        }
        
        std::cout << "Final game state - Players: " << worldMgr.getPlayerCount() 
                  << ", Game time: " << worldMgr.getGameTime() << std::endl;
        std::cout << std::endl;

        std::cout << "=== Comprehensive Demo Completed Successfully ===" << std::endl;
        std::cout << "All C++ features demonstrated and ready for Lua binding generation!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error during comprehensive demo: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "Unknown error during comprehensive demo" << std::endl;
        throw;
    }
}

void runLuaTests() {
    std::cout << "\n=== Running Lua Tests ===" << std::endl;
    
    // Initialize Lua state
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, 
                      sol::lib::math, sol::lib::table, sol::lib::io);
    
    try {
        // Register C++ bindings
        register_generated_module_bindings(lua);
        
        // Look for Lua test scripts in the scripts directory
        std::string scripts_dir = "examples/scripts";
        if (!std::filesystem::exists(scripts_dir)) {
            scripts_dir = "scripts"; // Try relative path
        }
        
        if (std::filesystem::exists(scripts_dir)) {
            std::cout << "Found scripts directory: " << scripts_dir << std::endl;
            
            // Test comprehensive features
            std::string script_path = scripts_dir + "/test_comprehensive.lua";
            if (std::filesystem::exists(script_path)) {
                std::cout << "\n--- Running test_comprehensive.lua ---" << std::endl;
                lua.script_file(script_path);
            } else {
                std::cout << "⚠️  Test script not found: " << script_path << std::endl;
            }
            
            // Test container factory
            std::string container_script_path = scripts_dir + "/container_creation_examples.lua";
            if (std::filesystem::exists(container_script_path)) {
                std::cout << "\n--- Running container_creation_examples.lua ---" << std::endl;
                lua.script_file(container_script_path);
            } else {
                std::cout << "⚠️  Container script not found: " << container_script_path << std::endl;
            }
            
            // Test full container factory features
            std::string full_container_script_path = scripts_dir + "/test_container_factory.lua";
            if (std::filesystem::exists(full_container_script_path)) {
                std::cout << "\n--- Running test_container_factory.lua ---" << std::endl;
                lua.script_file(full_container_script_path);
            } else {
                std::cout << "⚠️  Full container test script not found: " << full_container_script_path << std::endl;
            }
        } else {
            std::cout << "⚠️  Scripts directory not found" << std::endl;
        }
        
    } catch (const sol::error& e) {
        std::cout << "❌ Lua error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << std::endl;
    }
    
    std::cout << "\n=== Lua Tests Complete ===" << std::endl;
}

int main() {
    try {
        runComprehensiveDemo();
        runLuaTests();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}