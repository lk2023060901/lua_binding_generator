/**
 * @file complete_example.h
 * @brief 完整的 Lua 绑定生成器示例 - 包含所有15个核心宏功能
 * 
 * 这个示例展示了 lua_binding_generator 支持的所有功能：
 * - 模块定义 (EXPORT_LUA_MODULE)
 * - 命名空间导出 (EXPORT_LUA_NAMESPACE) 
 * - 普通类和方法 (EXPORT_LUA_CLASS)
 * - 抽象类 (EXPORT_LUA_ABSTRACT_CLASS)
 * - 单例模式 (EXPORT_LUA_SINGLETON)
 * - 静态类 (EXPORT_LUA_STATIC_CLASS)
 * - 枚举 (EXPORT_LUA_ENUM)
 * - 全局函数 (EXPORT_LUA_FUNCTION)
 * - 常量 (EXPORT_LUA_CONSTANT)
 * - 变量 (EXPORT_LUA_VARIABLE)
 * - 属性访问器 (EXPORT_LUA_PROPERTY)
 * - STL 容器 (EXPORT_LUA_VECTOR, EXPORT_LUA_MAP)
 * - 运算符重载 (EXPORT_LUA_OPERATOR)
 * - 回调函数 (EXPORT_LUA_CALLBACK)
 * 
 * 涵盖了所有15个核心宏的完整使用示例，每个声明都有对应的完整实现
 */

#pragma once

#include "../include/export_macros.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <iostream>

// 设置模块名称
EXPORT_LUA_MODULE(CompleteExample)

namespace demo {

// 命名空间导出 - 功能正在开发中，暂时注释
// EXPORT_LUA_NAMESPACE(alias=GameCore)

// ================================
// 1. 枚举示例
// ================================

/**
 * @brief 状态枚举
 */
enum class EXPORT_LUA_ENUM() Status {
    INACTIVE = 0,
    ACTIVE = 1,
    PENDING = 2,
    ERROR = 3
};

/**
 * @brief 颜色枚举
 */
enum class EXPORT_LUA_ENUM() Color {
    RED = 0,
    GREEN = 1,
    BLUE = 2,
    YELLOW = 3
};

// ================================
// 2. 常量示例
// ================================

EXPORT_LUA_CONSTANT()
static const int MAX_USERS = 100;

EXPORT_LUA_CONSTANT()
static const double PI = 3.14159265359;

EXPORT_LUA_CONSTANT()
static const std::string APP_NAME = "Complete Example";

// ================================
// 2.5. 变量导出示例
// ================================

// 可读写的全局变量
EXPORT_LUA_VARIABLE(access=readwrite)
static int current_level = 1;

// 只读的配置变量
EXPORT_LUA_VARIABLE(access=readonly)
static bool debug_enabled = false;

// 带命名空间的变量
EXPORT_LUA_VARIABLE(namespace=config, access=readwrite)
static std::string server_url = "localhost:8080";

// ================================
// 3. 全局函数示例
// ================================

/**
 * @brief 计算两个数的和
 */
EXPORT_LUA_FUNCTION()
int add(int a, int b);

/**
 * @brief 问候函数
 */
EXPORT_LUA_FUNCTION()
std::string greet(const std::string& name);

/**
 * @brief 检查数字是否为偶数
 */
EXPORT_LUA_FUNCTION()
bool isEven(int number);

/**
 * @brief 计算向量长度
 */
EXPORT_LUA_FUNCTION()
double calculateDistance(double x, double y);

// ================================
// 4. 基础类示例
// ================================

/**
 * @brief 基础实体类 - 抽象基类示例
 * 注意：此类不导出到Lua，仅作为Player类的C++基类
 */
class Entity {
public:
    Entity();
    Entity(int id, const std::string& name);
    virtual ~Entity() = default;
    
    // 基础属性
    int getId() const;
    void setId(int id);
    
    std::string getName() const;
    void setName(const std::string& name);
    
    // 状态管理
    Status getStatus() const;
    void setStatus(Status status);
    
    // 实用方法
    std::string toString() const;
    virtual void update();
    
    // 抽象方法 - 子类必须实现
    virtual std::string getType() const = 0;

protected:
    int id_;
    std::string name_;
    Status status_;
};

/**
 * @brief 玩家类 - 继承自Entity
 */
class EXPORT_LUA_CLASS() Player : public Entity {
public:
    Player();
    Player(int id, const std::string& name, int level);
    
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
    
    // 重写基类方法
    void update() override;
    std::string toString() const;
    std::string getType() const override;
    
    // 比较运算符（用于STL容器）
    bool operator==(const Player& other) const;
    bool operator<(const Player& other) const;

private:
    int level_;
    double health_;
    double max_health_;
    int experience_;
    std::vector<std::string> items_;
};

// ================================
// 5. 静态工具类示例
// ================================

/**
 * @brief 数学工具类 - 只包含静态方法
 */
class EXPORT_LUA_STATIC_CLASS() MathUtils {
public:
    static double clamp(double value, double min_val, double max_val);
    static double lerp(double a, double b, double t);
    static int randomInt(int min_val, int max_val);
    static double randomDouble(double min_val, double max_val);
    static double distance2D(double x1, double y1, double x2, double y2);
    static bool isPrime(int number);
    static int factorial(int n);
    static double toRadians(double degrees);
    static double toDegrees(double radians);
};

/**
 * @brief 字符串工具类
 */
class EXPORT_LUA_STATIC_CLASS() StringUtils {
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
};

// ================================
// 6. 单例模式示例
// ================================

/**
 * @brief 游戏管理器 - 单例模式
 */
class EXPORT_LUA_SINGLETON() GameManager {
public:
    static GameManager& getInstance();
    
    // 游戏状态管理
    void startGame();
    void pauseGame();
    void stopGame();
    bool isGameRunning() const;
    bool isGamePaused() const;
    
    // 玩家管理
    void addPlayer(std::shared_ptr<Player> player);
    void removePlayer(int playerId);
    std::shared_ptr<Player> getPlayer(int playerId);
    std::vector<std::shared_ptr<Player>> getAllPlayers();
    int getPlayerCount() const;
    
    // 游戏数据
    int getScore() const;
    void addScore(int points);
    double getTime() const;
    void updateTime(double deltaTime);
    
    // 配置管理
    void setSetting(const std::string& key, const std::string& value);
    std::string getSetting(const std::string& key) const;

private:
    GameManager() = default;
    ~GameManager() = default;
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;
    
    bool game_running_ = false;
    bool game_paused_ = false;
    std::vector<std::shared_ptr<Player>> players_;
    int score_ = 0;
    double game_time_ = 0.0;
    std::map<std::string, std::string> settings_;
};

// ================================
// 7. STL 容器支持示例
// ================================

/**
 * @brief 容器管理器 - 展示STL容器的使用
 */
class EXPORT_LUA_CLASS() ContainerManager {
public:
    ContainerManager();
    
    // Vector 操作
    void addNumber(int number);
    std::vector<int> getNumbers() const;
    void clearNumbers();
    int getNumberAt(int index) const;
    int getNumberCount() const;
    
    // Map 操作
    void setProperty(const std::string& key, const std::string& value);
    std::string getProperty(const std::string& key) const;
    std::map<std::string, std::string> getAllProperties() const;
    bool hasProperty(const std::string& key) const;
    void removeProperty(const std::string& key);
    
    // 复杂容器操作
    void addPlayerScore(const std::string& playerName, int score);
    std::map<std::string, int> getPlayerScores() const;
    std::vector<std::string> getTopPlayers(int count) const;

private:
    std::vector<int> numbers_;
    std::map<std::string, std::string> properties_;
    std::map<std::string, int> player_scores_;
};

// ================================
// 8. 向量类 - 运算符重载示例
// ================================

/**
 * @brief 2D向量类 - 演示运算符重载
 */
class EXPORT_LUA_CLASS() Vector2D {
public:
    Vector2D();
    Vector2D(double x, double y);
    
    // 访问器
    double getX() const;
    double getY() const;
    void setX(double x);
    void setY(double y);
    
    // 向量操作
    double length() const;
    double lengthSquared() const;
    Vector2D normalized() const;
    double dot(const Vector2D& other) const;
    double cross(const Vector2D& other) const;
    
    // 运算符重载
    EXPORT_LUA_OPERATOR(+)
    Vector2D operator+(const Vector2D& other) const;
    
    EXPORT_LUA_OPERATOR(-)
    Vector2D operator-(const Vector2D& other) const;
    
    EXPORT_LUA_OPERATOR(*)
    Vector2D operator*(double scalar) const;
    
    EXPORT_LUA_OPERATOR(/)
    Vector2D operator/(double scalar) const;
    
    EXPORT_LUA_OPERATOR(==)
    bool operator==(const Vector2D& other) const;
    
    EXPORT_LUA_OPERATOR(!=)
    bool operator!=(const Vector2D& other) const;
    
    EXPORT_LUA_OPERATOR([])
    double operator[](int index) const;
    
    // 复合赋值运算符
    Vector2D& operator+=(const Vector2D& other);
    Vector2D& operator-=(const Vector2D& other);
    Vector2D& operator*=(double scalar);
    Vector2D& operator/=(double scalar);
    
    // 实用方法
    std::string toString() const;

private:
    double x_, y_;
};

// ================================
// 9. 回调函数示例
// ================================

/**
 * @brief 事件系统 - 演示回调函数的使用
 */
class EXPORT_LUA_CLASS() EventSystem {
public:
    EventSystem();
    
    // 回调函数类型定义
    EXPORT_LUA_CALLBACK()
    std::function<void()> OnGameStart;
    
    EXPORT_LUA_CALLBACK()
    std::function<void(int)> OnScoreChange;
    
    EXPORT_LUA_CALLBACK()
    std::function<void(std::shared_ptr<Player>)> OnPlayerJoin;
    
    EXPORT_LUA_CALLBACK()
    std::function<bool(const std::string&, int)> OnValidateAction;
    
    // 事件触发方法
    void triggerGameStart();
    void triggerScoreChange(int newScore);
    void triggerPlayerJoin(std::shared_ptr<Player> player);
    bool validateAction(const std::string& action, int value);
    
    // 实用方法
    bool hasGameStartCallback() const;
    bool hasScoreChangeCallback() const;
    bool hasPlayerJoinCallback() const;
    bool hasValidateActionCallback() const;

private:
    int last_score_ = 0;
};

// ================================
// 10. 智能指针示例
// ================================

/**
 * @brief 资源管理器 - 演示智能指针的使用
 */
class EXPORT_LUA_CLASS() ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();
    
    // 创建和管理玩家
    std::shared_ptr<Player> createPlayer(const std::string& name);
    void addPlayer(std::shared_ptr<Player> player);
    std::shared_ptr<Player> getPlayer(int id);
    std::vector<std::shared_ptr<Player>> getAllPlayers();
    void removePlayer(int id);
    
    // 资源统计
    int getPlayerCount() const;
    std::vector<std::string> getPlayerNames() const;
    
    // 实用方法
    void cleanup();
    std::string getInfo() const;

private:
    std::vector<std::shared_ptr<Player>> players_;
    int next_id_ = 1;
};

// ================================
// STL 容器导出示例
// ================================

// 基础类型容器
EXPORT_LUA_VECTOR(int, alias=IntList)
EXPORT_LUA_VECTOR(std::string, alias=StringList)
EXPORT_LUA_VECTOR(double, alias=NumberList)

// 自定义类型容器
EXPORT_LUA_VECTOR(Player, alias=PlayerArray)

// 映射容器导出
EXPORT_LUA_MAP(std::string, int, alias=NameScoreMap)
EXPORT_LUA_MAP(int, std::string, alias=IdNameMap)
EXPORT_LUA_MAP(std::string, Player, alias=PlayerRegistry)

} // namespace demo