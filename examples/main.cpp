/**
 * @file main.cpp
 * @brief 完整示例的主程序 - 测试生成的Lua绑定
 */

#include "complete_example.h"
#include <sol/sol.hpp>
#include <iostream>

// 声明绑定注册函数
extern void register_generated_module_bindings(sol::state& lua);

void testBasicFunctions(sol::state& lua) {
    std::cout << "\n=== 测试基础函数 ===" << std::endl;
    
    // 测试全局函数
    int sum = lua["add"](5, 3);
    std::cout << "add(5, 3) = " << sum << std::endl;
    
    std::string greeting = lua["greet"]("World");
    std::cout << "greet('World') = " << greeting << std::endl;
    
    bool even = lua["isEven"](4);
    std::cout << "isEven(4) = " << (even ? "true" : "false") << std::endl;
    
    double distance = lua["calculateDistance"](3.0, 4.0);
    std::cout << "calculateDistance(3, 4) = " << distance << std::endl;
}

void testConstants(sol::state& lua) {
    std::cout << "\n=== 测试常量 ===" << std::endl;
    
    // 使用 C++ 常量直接测试
    std::cout << "MAX_USERS = " << demo::MAX_USERS << std::endl;
    std::cout << "PI = " << demo::PI << std::endl;
    std::cout << "APP_NAME = " << demo::APP_NAME << std::endl;
    
    // 测试通过 Lua 访问常量
    lua.script(R"(
        print("从 Lua 访问常量:")
        if demo.MAX_USERS then print("MAX_USERS = " .. demo.MAX_USERS) else print("MAX_USERS not bound") end
        if demo.PI then print("PI = " .. demo.PI) else print("PI not bound") end
        if demo.APP_NAME then print("APP_NAME = " .. demo.APP_NAME) else print("APP_NAME not bound") end
    )");
}

void testClasses(sol::state& lua) {
    std::cout << "\n=== 测试类功能 ===" << std::endl;
    
    // 使用 C++ 对象直接测试
    // Note: Entity is abstract, using Player which inherits from Entity
    demo::Player entity_as_player(1, "TestEntity", 1);
    std::cout << "Created entity (as player): " << entity_as_player.toString() << std::endl;
    
    demo::Player player(2, "TestPlayer", 5);
    std::cout << "Created player: " << player.toString() << std::endl;
    
    // 测试方法调用
    player.addExperience(150);
    int level = player.getLevel();
    std::cout << "Player level after adding 150 exp: " << level << std::endl;
    
    // 测试通过 Lua 调用
    lua.script(R"(
        print("从 Lua 创建对象:")
        print("demo table type:", type(demo))
        
        -- 注意：Entity是抽象基类，不导出到Lua
        -- 我们直接测试Player类，它继承自Entity
        print("demo.Player type:", type(demo.Player))
        if demo.Player then
            print("demo.Player exists")
            local player = demo.Player.new()
            print("Player created successfully")
            print("Player ID:", player:getId())
            print("Player name:", player:getName())
            print("Player type:", player:getType())
        else
            print("demo.Player does not exist")
        end
    )");
}

void testStaticClasses(sol::state& lua) {
    std::cout << "\n=== 测试静态类 ===" << std::endl;
    
    // 使用 C++ 静态方法直接测试
    double clamped = demo::MathUtils::clamp(150.0, 0.0, 100.0);
    std::cout << "MathUtils.clamp(150, 0, 100) = " << clamped << std::endl;
    
    bool prime = demo::MathUtils::isPrime(17);
    std::cout << "MathUtils.isPrime(17) = " << (prime ? "true" : "false") << std::endl;
    
    std::string upper = demo::StringUtils::toUpper("hello world");
    std::cout << "StringUtils.toUpper('hello world') = " << upper << std::endl;
    
    // 测试通过 Lua 调用静态方法
    lua.script(R"(
        print("从 Lua 调用静态方法:")
        if demo.MathUtils and demo.MathUtils.clamp then
            print("MathUtils.clamp(200, 0, 100) = " .. demo.MathUtils.clamp(200, 0, 100))
        else
            print("静态类方法尚未绑定")
        end
    )");
}

void testSingleton(sol::state& lua) {
    std::cout << "\n=== 测试单例模式 ===" << std::endl;
    
    // 使用 C++ 单例直接测试
    auto& gameManager = demo::GameManager::getInstance();
    gameManager.startGame();
    gameManager.addScore(100);
    
    bool running = gameManager.isGameRunning();
    int score = gameManager.getScore();
    
    std::cout << "Game running: " << (running ? "true" : "false") << std::endl;
    std::cout << "Current score: " << score << std::endl;
    
    // 测试通过 Lua 调用单例
    lua.script(R"(
        print("从 Lua 调用单例:")
        if demo.GameManager and demo.GameManager.getInstance then
            local gm = demo.GameManager.getInstance()
            if gm and gm.getScore then
                print("Current score from Lua: " .. gm:getScore())
            else
                print("单例方法尚未绑定")
            end
        else
            print("单例类尚未绑定")
        end
    )");
}

void testContainers(sol::state& lua) {
    std::cout << "\n=== 测试容器管理 ===" << std::endl;
    
    // 使用 C++ 对象直接测试
    demo::ContainerManager containerManager;
    
    // 测试数字容器
    containerManager.addNumber(10);
    containerManager.addNumber(20);
    containerManager.addNumber(30);
    
    int count = containerManager.getNumberCount();
    std::cout << "Number count: " << count << std::endl;
    
    // 测试属性容器
    containerManager.setProperty("name", "TestContainer");
    containerManager.setProperty("version", "1.0");
    
    std::string name = containerManager.getProperty("name");
    std::cout << "Property 'name': " << name << std::endl;
    
    // 测试通过 Lua 调用
    lua.script(R"(
        print("从 Lua 创建容器管理器:")
        local cm = demo.ContainerManager.new()
        if cm and cm.addNumber then
            cm:addNumber(100)
            cm:addNumber(200)
            print("Numbers added from Lua: " .. cm:getNumberCount())
        else
            print("容器管理器方法尚未绑定")
        end
    )");
}

void testVectors(sol::state& lua) {
    std::cout << "\n=== 测试向量运算 ===" << std::endl;
    
    // 创建 Vector2D 对象
    sol::table demo_ns = lua["demo"];
    sol::usertype<demo::Vector2D> vector_type = demo_ns["Vector2D"];
    
    // 创建向量实例
    demo::Vector2D vec1(3.0, 4.0);
    demo::Vector2D vec2(1.0, 2.0);
    
    // 测试向量运算
    demo::Vector2D sum = vec1 + vec2;
    demo::Vector2D product = vec1 * 2.0;
    
    double length = vec1.length();
    bool equal = vec1 == vec2;
    
    std::cout << "vec1.length() = " << length << std::endl;
    std::cout << "vec1 == vec2: " << (equal ? "true" : "false") << std::endl;
    std::cout << "vec1 + vec2 = " << sum.toString() << std::endl;
    std::cout << "vec1 * 2 = " << product.toString() << std::endl;
    
    // 测试通过 Lua 调用
    lua.script(R"(
        local vec1 = demo.Vector2D.new()
        vec1:setX(3.0)
        vec1:setY(4.0)
        local vec2 = demo.Vector2D.new()
        vec2:setX(1.0)
        vec2:setY(2.0)
        print("从 Lua 调用:")
        print("vec1:toString() = " .. vec1:toString())
        print("vec2:toString() = " .. vec2:toString())
        print("vec1:length() = " .. vec1:length())
    )");
}

void testSTLContainers(sol::state& lua) {
    std::cout << "\n=== 测试 STL 容器操作 ===" << std::endl;
    
    // 测试在 Lua 中创建和操作 STL 容器
    lua.script(R"(
        print("=== 在 Lua 中创建和操作 STL 容器 ===")
        
        -- 1. 测试 IntList (std::vector<int>)
        print("\n--- IntList 测试 ---")
        local int_list = IntList.new()
        print("创建空 IntList, 大小:", int_list:size())
        print("是否为空:", int_list:empty())
        
        -- 添加元素
        int_list:push_back(10)
        int_list:push_back(20)
        int_list:push_back(30)
        print("添加 10, 20, 30 后，大小:", int_list:size())
        print("是否为空:", int_list:empty())
        
        -- 删除最后一个元素
        int_list:pop_back()
        print("删除最后一个元素后，大小:", int_list:size())
        
        -- 清空容器
        int_list:clear()
        print("清空后，大小:", int_list:size())
        print("是否为空:", int_list:empty())
        
        -- 2. 测试 StringList (std::vector<std::string>)  
        print("\n--- StringList 测试 ---")
        local string_list = StringList.new()
        print("创建空 StringList, 大小:", string_list:size())
        
        -- 添加字符串
        string_list:push_back("Hello")
        string_list:push_back("World") 
        string_list:push_back("From Lua")
        print("添加字符串后，大小:", string_list:size())
        
        -- 删除一个元素
        string_list:pop_back()
        print("删除最后一个元素后，大小:", string_list:size())
        
        -- 3. 测试 NumberList (std::vector<double>)
        print("\n--- NumberList 测试 ---")
        local number_list = NumberList.new()
        number_list:push_back(3.14)
        number_list:push_back(2.71)
        number_list:push_back(1.41)
        print("添加浮点数后，大小:", number_list:size())
        
        -- 清空测试
        number_list:clear()
        print("清空后，大小:", number_list:size())
        
        -- 4. 测试 PlayerArray (std::vector<demo::Player>)
        print("\n--- PlayerArray 测试 ---")
        local player_array = PlayerArray.new()
        print("创建空 PlayerArray, 大小:", player_array:size())
        
        -- 创建玩家并添加到数组
        local player1 = demo.Player.new()
        local player2 = demo.Player.new()
        
        player_array:push_back(player1)
        player_array:push_back(player2)
        print("添加 2 个玩家后，大小:", player_array:size())
        
        -- 删除一个玩家
        player_array:pop_back()
        print("删除一个玩家后，大小:", player_array:size())
        
        -- 清空玩家数组
        player_array:clear()
        print("清空玩家数组后，大小:", player_array:size())
        
        -- 5. 综合测试 - 错误处理
        print("\n--- 错误处理测试 ---")
        local test_list = IntList.new()
        
        -- 测试在空容器上调用 pop_back
        print("在空容器上调用 pop_back（应该安全）")
        test_list:pop_back()  -- 应该安全，不会崩溃
        print("操作完成，容器大小:", test_list:size())
        
        -- 测试大量添加
        print("测试添加大量元素:")
        for i = 1, 100 do
            test_list:push_back(i)
        end
        print("添加 100 个元素后，大小:", test_list:size())
        
        -- 测试大量删除
        print("测试删除所有元素:")
        while not test_list:empty() do
            test_list:pop_back()
        end
        print("删除完成，最终大小:", test_list:size())
        
        -- 6. 测试 Map 容器
        print("\n=== Map 容器测试 ===")
        
        -- 测试 NameScoreMap (std::map<std::string, int>)
        print("\n--- NameScoreMap 测试 ---")
        local name_score_map = NameScoreMap.new()
        print("创建空 NameScoreMap, 大小:", name_score_map:size())
        print("是否为空:", name_score_map:empty())
        
        -- 添加键值对
        name_score_map:set("Alice", 100)
        name_score_map:set("Bob", 85)
        name_score_map:set("Charlie", 92)
        print("添加 3 个键值对后，大小:", name_score_map:size())
        print("是否为空:", name_score_map:empty())
        
        -- 访问和检查
        local alice_score = name_score_map:get("Alice")
        print("Alice 的分数:", alice_score and alice_score or "不存在")
        print("David 存在吗:", name_score_map:has("David"))
        print("Bob 存在吗:", name_score_map:has("Bob"))
        
        -- 获取所有键和值
        local all_keys = name_score_map:keys()
        local all_values = name_score_map:values()
        print("所有键的数量:", #all_keys)
        print("所有值的数量:", #all_values)
        
        -- 删除键
        local removed = name_score_map:erase("Bob")
        print("删除 Bob:", removed and "成功" or "失败")
        print("删除后，大小:", name_score_map:size())
        
        -- 清空
        name_score_map:clear()
        print("清空后，大小:", name_score_map:size())
        
        -- 测试 IdNameMap (std::map<int, std::string>)
        print("\n--- IdNameMap 测试 ---")
        local id_name_map = IdNameMap.new()
        id_name_map:set(1, "First")
        id_name_map:set(2, "Second")
        id_name_map:set(3, "Third")
        print("添加 3 个映射后，大小:", id_name_map:size())
        
        local name_2 = id_name_map:get(2)
        print("ID 2 对应的名字:", name_2 and name_2 or "不存在")
        print("ID 5 存在吗:", id_name_map:has(5))
        
        -- 测试 PlayerRegistry (std::map<std::string, demo::Player>)
        print("\n--- PlayerRegistry 测试 ---")
        local player_registry = PlayerRegistry.new()
        print("创建空 PlayerRegistry, 大小:", player_registry:size())
        
        -- 创建玩家并添加到注册表
        local player1 = demo.Player.new()
        local player2 = demo.Player.new()
        
        player_registry:set("hero", player1)
        player_registry:set("villain", player2)
        print("添加 2 个玩家后，大小:", player_registry:size())
        
        -- 检查玩家是否存在
        print("hero 存在吗:", player_registry:has("hero"))
        print("neutral 存在吗:", player_registry:has("neutral"))
        
        -- 获取玩家
        local hero = player_registry:get("hero")
        if hero then
            print("成功获取 hero 玩家")
        else
            print("获取 hero 玩家失败")
        end
        
        -- 删除玩家
        local removed_player = player_registry:erase("villain")
        print("删除 villain:", removed_player and "成功" or "失败")
        print("删除后，大小:", player_registry:size())
        
        -- 清空注册表
        player_registry:clear()
        print("清空后，大小:", player_registry:size())
        
        print("=== Map 容器测试完成 ===")
        print("=== STL 容器测试完成 ===")
    )");
}

int main() {
    try {
        std::cout << "=== Lua 绑定生成器完整示例测试 ===" << std::endl;
        
        // 创建Lua状态
        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string);
        
        // 注册生成的绑定
        register_generated_module_bindings(lua);
        
        // 运行测试
        testBasicFunctions(lua);
        testConstants(lua);
        testClasses(lua);
        testStaticClasses(lua);
        testSingleton(lua);
        testContainers(lua);
        testVectors(lua);
        testSTLContainers(lua);
        
        std::cout << "\n=== 所有测试完成 ===" << std::endl;
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
}