/**
 * @file macro_coverage.h
 * @brief 15个核心宏的100%覆盖测试
 * 
 * 这个文件专门设计用于测试lua_binding_generator的所有核心宏功能，
 * 确保每个宏都有完整的测试覆盖，包括各种使用场景和边界情况。
 * 
 * 核心宏列表（按优先级排序）：
 * 1.  EXPORT_LUA_MODULE        - 模块定义
 * 2.  EXPORT_LUA_NAMESPACE     - 命名空间导出
 * 3.  EXPORT_LUA_CLASS         - 普通类
 * 4.  EXPORT_LUA_ENUM          - 枚举
 * 5.  EXPORT_LUA_SINGLETON     - 单例模式
 * 6.  EXPORT_LUA_STATIC_CLASS  - 静态类
 * 7.  EXPORT_LUA_ABSTRACT_CLASS- 抽象类
 * 8.  EXPORT_LUA_FUNCTION      - 全局函数
 * 9.  EXPORT_LUA_VARIABLE      - 变量
 * 10. EXPORT_LUA_CONSTANT      - 常量
 * 11. EXPORT_LUA_VECTOR        - Vector容器
 * 12. EXPORT_LUA_MAP           - Map容器
 * 13. EXPORT_LUA_CALLBACK      - 回调函数
 * 14. EXPORT_LUA_OPERATOR      - 运算符重载
 * 15. EXPORT_LUA_PROPERTY      - 属性访问器
 */

#pragma once

#include "export_macros.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <iostream>
#include <cmath>

// ================================
// 1. EXPORT_LUA_MODULE - 模块定义测试
// ================================

/**
 * 测试模块定义宏，设置整个文件的默认模块
 */
EXPORT_LUA_MODULE(MacroCoverageTest)

// ================================
// 2. EXPORT_LUA_NAMESPACE - 命名空间导出测试
// ================================

/**
 * 测试命名空间导出，这将创建一个Lua命名空间
 */
namespace EXPORT_LUA_NAMESPACE(alias=TestCoverage) test_coverage {

// ================================
// 4. EXPORT_LUA_ENUM - 枚举测试（优先级较高）
// ================================

/**
 * @brief 基础枚举测试
 */
enum class EXPORT_LUA_ENUM() TestStatus {
    INACTIVE = 0,
    ACTIVE = 1,
    PENDING = 2,
    COMPLETED = 3,
    ERROR = -1
};

/**
 * @brief 复杂枚举测试（带自定义值）
 */
enum class EXPORT_LUA_ENUM() TestPriority {
    LOW = 1,
    MEDIUM = 5,
    HIGH = 10,
    CRITICAL = 100
};

/**
 * @brief 传统枚举测试（非enum class）
 */
enum EXPORT_LUA_ENUM() TestFlags {
    FLAG_NONE = 0,
    FLAG_READ = 1,
    FLAG_WRITE = 2,
    FLAG_EXECUTE = 4,
    FLAG_ALL = FLAG_READ | FLAG_WRITE | FLAG_EXECUTE
};

// ================================
// 10. EXPORT_LUA_CONSTANT - 常量测试
// ================================

/**
 * 整数常量测试
 */
EXPORT_LUA_CONSTANT()
static const int MAX_CONNECTIONS = 1000;

/**
 * 浮点常量测试
 */
EXPORT_LUA_CONSTANT()
static const double PI_VALUE = 3.14159265359;

/**
 * 字符串常量测试
 */
EXPORT_LUA_CONSTANT()
static const std::string TEST_VERSION = "2.0.0";

/**
 * 布尔常量测试
 */
EXPORT_LUA_CONSTANT()
static const bool DEBUG_ENABLED = true;

// ================================
// 9. EXPORT_LUA_VARIABLE - 变量测试
// ================================

/**
 * 可读写全局变量
 */
EXPORT_LUA_VARIABLE(access=readwrite)
static int global_counter = 0;

/**
 * 只读全局变量
 */
EXPORT_LUA_VARIABLE(access=readonly)
static std::string system_name = "MacroCoverageTest";

/**
 * 命名空间变量测试
 */
EXPORT_LUA_VARIABLE(namespace=config, access=readwrite)
static double global_multiplier = 1.0;

// ================================
// 8. EXPORT_LUA_FUNCTION - 全局函数测试
// ================================

/**
 * @brief 简单数学函数测试
 */
EXPORT_LUA_FUNCTION()
int add_numbers(int a, int b);

/**
 * @brief 字符串处理函数测试
 */
EXPORT_LUA_FUNCTION()
std::string format_message(const std::string& template_str, const std::string& value);

/**
 * @brief 复杂参数函数测试
 */
EXPORT_LUA_FUNCTION()
std::vector<int> generate_sequence(int start, int end, int step = 1);

/**
 * @brief 重载函数测试
 */
EXPORT_LUA_FUNCTION()
double calculate_area(double radius);

EXPORT_LUA_FUNCTION()
double calculate_area(double width, double height);

/**
 * @brief 回调参数函数测试
 */
EXPORT_LUA_FUNCTION()
void process_items(const std::vector<int>& items, std::function<void(int)> processor);

// ================================
// 7. EXPORT_LUA_ABSTRACT_CLASS - 抽象类测试
// ================================

/**
 * @brief 抽象基类测试
 * 注意：抽象类不能直接在Lua中实例化，但可以作为基类使用
 */
class EXPORT_LUA_ABSTRACT_CLASS() TestEntity {
public:
    TestEntity();
    TestEntity(int id, const std::string& name);
    virtual ~TestEntity() = default;
    
    // 基础属性
    int getId() const;
    void setId(int id);
    
    std::string getName() const;
    void setName(const std::string& name);
    
    TestStatus getStatus() const;
    void setStatus(TestStatus status);
    
    // 实用方法
    std::string toString() const;
    virtual void update();
    
    // 纯虚函数 - 派生类必须实现
    virtual std::string getType() const = 0;
    virtual double getScore() const = 0;

protected:
    int id_;
    std::string name_;
    TestStatus status_;
};

// ================================
// 3. EXPORT_LUA_CLASS - 普通类测试
// ================================

/**
 * @brief 基础类测试（继承自抽象类）
 */
class EXPORT_LUA_CLASS() TestPlayer : public TestEntity {
public:
    TestPlayer();
    TestPlayer(const std::string& name, int level = 1);
    TestPlayer(int id, const std::string& name, int level);
    
    // 级别管理
    EXPORT_LUA_PROPERTY(access=readonly)
    int getLevel() const;
    void setLevel(int level);
    void levelUp();
    
    // 血量管理 - 属性导出示例
    EXPORT_LUA_PROPERTY(access=readwrite, setter=setHealth)
    double getHealth() const;
    void setHealth(double health);
    
    EXPORT_LUA_PROPERTY(access=readonly)
    double getMaxHealth() const;
    void heal(double amount);
    void takeDamage(double amount);
    
    // 经验值管理
    EXPORT_LUA_PROPERTY(access=readonly)
    int getExperience() const;
    void addExperience(int exp);
    
    // 装备管理
    void addItem(const std::string& item);
    std::vector<std::string> getItems() const;
    bool hasItem(const std::string& item) const;
    void removeItem(const std::string& item);
    
    // 重写基类方法
    void update() override;
    std::string getType() const override;
    double getScore() const override;
    
    // 比较运算符（用于STL容器）
    bool operator==(const TestPlayer& other) const;
    bool operator<(const TestPlayer& other) const;
    
    // 友元函数测试
    friend std::ostream& operator<<(std::ostream& os, const TestPlayer& player);

private:
    int level_;
    double health_;
    double max_health_;
    int experience_;
    std::vector<std::string> items_;
};

/**
 * @brief 复杂类测试（多重继承、模板等）
 */
class EXPORT_LUA_CLASS() TestManager {
public:
    TestManager();
    ~TestManager();
    
    // 玩家管理
    void addPlayer(std::shared_ptr<TestPlayer> player);
    void removePlayer(int playerId);
    std::shared_ptr<TestPlayer> getPlayer(int playerId);
    std::vector<std::shared_ptr<TestPlayer>> getAllPlayers();
    int getPlayerCount() const;
    
    // 统计功能
    double getAverageLevel() const;
    TestPlayer* getTopPlayer() const;
    std::vector<std::string> getPlayerNames() const;
    
    // 批量操作
    void levelUpAll();
    void healAll(double amount);
    void clearAll();

private:
    std::vector<std::shared_ptr<TestPlayer>> players_;
    int next_id_;
};

// ================================
// 6. EXPORT_LUA_STATIC_CLASS - 静态类测试
// ================================

/**
 * @brief 数学工具静态类测试
 */
class EXPORT_LUA_STATIC_CLASS() TestMathUtils {
public:
    static double clamp(double value, double min_val, double max_val);
    static double lerp(double a, double b, double t);
    static int randomInt(int min_val, int max_val);
    static double randomDouble(double min_val = 0.0, double max_val = 1.0);
    static double distance2D(double x1, double y1, double x2, double y2);
    static double distance3D(double x1, double y1, double z1, double x2, double y2, double z2);
    static bool isPrime(int number);
    static int factorial(int n);
    static double toRadians(double degrees);
    static double toDegrees(double radians);
    static std::vector<int> generatePrimes(int max_num);
};

/**
 * @brief 字符串工具静态类测试
 */
class EXPORT_LUA_STATIC_CLASS() TestStringUtils {
public:
    static std::string toUpper(const std::string& str);
    static std::string toLower(const std::string& str);
    static std::string trim(const std::string& str);
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string join(const std::vector<std::string>& parts, const std::string& separator);
    static bool startsWith(const std::string& str, const std::string& prefix);
    static bool endsWith(const std::string& str, const std::string& suffix);
    static std::string reverse(const std::string& str);
    static int count(const std::string& str, char ch);
    static std::string replace(const std::string& str, const std::string& from, const std::string& to);
};

// ================================
// 5. EXPORT_LUA_SINGLETON - 单例模式测试
// ================================

/**
 * @brief 测试管理器单例类
 */
class EXPORT_LUA_SINGLETON() TestGameManager {
public:
    static TestGameManager& getInstance();
    
    // 游戏状态管理
    void startGame();
    void pauseGame();
    void stopGame();
    bool isGameRunning() const;
    bool isGamePaused() const;
    
    // 玩家管理
    void addPlayer(std::shared_ptr<TestPlayer> player);
    void removePlayer(int playerId);
    std::shared_ptr<TestPlayer> getPlayer(int playerId);
    std::vector<std::shared_ptr<TestPlayer>> getAllPlayers();
    int getPlayerCount() const;
    
    // 游戏数据
    int getScore() const;
    void addScore(int points);
    double getTime() const;
    void updateTime(double deltaTime);
    void resetGame();
    
    // 配置管理
    void setSetting(const std::string& key, const std::string& value);
    std::string getSetting(const std::string& key) const;
    bool hasSetting(const std::string& key) const;
    void clearSettings();

private:
    TestGameManager() = default;
    ~TestGameManager() = default;
    TestGameManager(const TestGameManager&) = delete;
    TestGameManager& operator=(const TestGameManager&) = delete;
    
    bool game_running_ = false;
    bool game_paused_ = false;
    std::vector<std::shared_ptr<TestPlayer>> players_;
    int score_ = 0;
    double game_time_ = 0.0;
    std::map<std::string, std::string> settings_;
};

// ================================
// 14. EXPORT_LUA_OPERATOR - 运算符重载测试
// ================================

/**
 * @brief 向量类 - 演示运算符重载
 */
class EXPORT_LUA_CLASS() TestVector2D {
public:
    TestVector2D();
    TestVector2D(double x, double y);
    TestVector2D(const TestVector2D& other);
    
    // 访问器
    double getX() const;
    double getY() const;
    void setX(double x);
    void setY(double y);
    
    // 向量操作
    double length() const;
    double lengthSquared() const;
    TestVector2D normalized() const;
    double dot(const TestVector2D& other) const;
    double cross(const TestVector2D& other) const;
    double distance(const TestVector2D& other) const;
    
    // 运算符重载 - 所有主要运算符
    EXPORT_LUA_OPERATOR(+)
    TestVector2D operator+(const TestVector2D& other) const;
    
    EXPORT_LUA_OPERATOR(-)
    TestVector2D operator-(const TestVector2D& other) const;
    
    EXPORT_LUA_OPERATOR(*)
    TestVector2D operator*(double scalar) const;
    
    EXPORT_LUA_OPERATOR(/)
    TestVector2D operator/(double scalar) const;
    
    EXPORT_LUA_OPERATOR(==)
    bool operator==(const TestVector2D& other) const;
    
    EXPORT_LUA_OPERATOR(!=)
    bool operator!=(const TestVector2D& other) const;
    
    EXPORT_LUA_OPERATOR(<)
    bool operator<(const TestVector2D& other) const;
    
    EXPORT_LUA_OPERATOR([])
    double operator[](int index) const;
    
    // 复合赋值运算符
    TestVector2D& operator+=(const TestVector2D& other);
    TestVector2D& operator-=(const TestVector2D& other);
    TestVector2D& operator*=(double scalar);
    TestVector2D& operator/=(double scalar);
    
    // 一元运算符
    EXPORT_LUA_OPERATOR(-)
    TestVector2D operator-() const;
    
    // 赋值运算符
    TestVector2D& operator=(const TestVector2D& other);
    
    // 实用方法
    std::string toString() const;
    void zero();
    void normalize();

private:
    double x_, y_;
};

// ================================
// 13. EXPORT_LUA_CALLBACK - 回调函数测试
// ================================

/**
 * @brief 事件系统 - 演示回调函数的使用
 */
class EXPORT_LUA_CLASS() TestEventSystem {
public:
    TestEventSystem();
    ~TestEventSystem();
    
    // 回调函数类型定义 - 各种签名的回调
    EXPORT_LUA_CALLBACK()
    std::function<void()> OnGameStart;
    
    EXPORT_LUA_CALLBACK()
    std::function<void(int)> OnScoreChange;
    
    EXPORT_LUA_CALLBACK()
    std::function<void(std::shared_ptr<TestPlayer>)> OnPlayerJoin;
    
    EXPORT_LUA_CALLBACK()
    std::function<bool(const std::string&, int)> OnValidateAction;
    
    EXPORT_LUA_CALLBACK()
    std::function<void(double, double)> OnPositionChange;
    
    EXPORT_LUA_CALLBACK()
    std::function<std::string(const std::string&)> OnMessageFilter;
    
    // 事件触发方法
    void triggerGameStart();
    void triggerScoreChange(int newScore);
    void triggerPlayerJoin(std::shared_ptr<TestPlayer> player);
    bool validateAction(const std::string& action, int value);
    void triggerPositionChange(double x, double y);
    std::string filterMessage(const std::string& message);
    
    // 回调管理
    bool hasGameStartCallback() const;
    bool hasScoreChangeCallback() const;
    bool hasPlayerJoinCallback() const;
    bool hasValidateActionCallback() const;
    
    void clearAllCallbacks();
    
    // 批量事件处理
    void triggerMultipleEvents();

private:
    int last_score_ = 0;
    double last_x_ = 0.0, last_y_ = 0.0;
};

// ================================
// 11-12. STL 容器导出测试
// ================================

/**
 * @brief 容器管理类 - 演示STL容器的导出
 */
class EXPORT_LUA_CLASS() TestContainerManager {
public:
    TestContainerManager();
    ~TestContainerManager();
    
    // Vector 操作测试
    void addNumber(int number);
    void addNumbers(const std::vector<int>& numbers);
    std::vector<int> getNumbers() const;
    void clearNumbers();
    int getNumberAt(int index) const;
    int getNumberCount() const;
    void removeNumberAt(int index);
    
    // String Vector 操作
    void addString(const std::string& str);
    std::vector<std::string> getStrings() const;
    std::string getStringAt(int index) const;
    
    // Map 操作测试
    void setProperty(const std::string& key, const std::string& value);
    std::string getProperty(const std::string& key) const;
    std::map<std::string, std::string> getAllProperties() const;
    bool hasProperty(const std::string& key) const;
    void removeProperty(const std::string& key);
    std::vector<std::string> getPropertyKeys() const;
    
    // 复杂容器操作
    void addPlayerScore(const std::string& playerName, int score);
    std::map<std::string, int> getPlayerScores() const;
    std::vector<std::string> getTopPlayers(int count) const;
    int getPlayerScore(const std::string& playerName) const;
    
    // Player 容器操作
    void addPlayerToList(std::shared_ptr<TestPlayer> player);
    std::vector<std::shared_ptr<TestPlayer>> getPlayerList() const;
    void clearPlayerList();

private:
    std::vector<int> numbers_;
    std::vector<std::string> strings_;
    std::map<std::string, std::string> properties_;
    std::map<std::string, int> player_scores_;
    std::vector<std::shared_ptr<TestPlayer>> player_list_;
};

// ================================
// STL 容器直接导出测试
// ================================

// 基础类型容器
EXPORT_LUA_VECTOR(int, alias=TestIntList)
EXPORT_LUA_VECTOR(std::string, alias=TestStringList)
EXPORT_LUA_VECTOR(double, alias=TestDoubleList)

// 自定义类型容器
EXPORT_LUA_VECTOR(TestPlayer, alias=TestPlayerArray)
EXPORT_LUA_VECTOR(std::shared_ptr<TestPlayer>, alias=TestPlayerPtrArray)

// Map 容器导出
EXPORT_LUA_MAP(std::string, int, alias=TestNameScoreMap)
EXPORT_LUA_MAP(int, std::string, alias=TestIdNameMap)
EXPORT_LUA_MAP(std::string, TestPlayer, alias=TestPlayerRegistry)
EXPORT_LUA_MAP(std::string, std::shared_ptr<TestPlayer>, alias=TestPlayerPtrRegistry)

// 复杂嵌套容器
EXPORT_LUA_VECTOR(std::vector<int>, alias=TestNestedIntList)
EXPORT_LUA_MAP(std::string, std::vector<int>, alias=TestGroupDataMap)

} // namespace test_coverage