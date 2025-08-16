/**
 * @file game_engine.h
 * @brief 游戏引擎示例 - 完整的游戏系统
 * 
 * 这个示例展示了一个完整的小型游戏引擎架构，
 * 包含实体系统、组件系统、事件系统、资源管理等
 */

#pragma once

#include "../include/export_macros.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <chrono>

EXPORT_LUA_MODULE(GameEngine)

namespace engine {
namespace core {

/**
 * @brief 游戏对象类型枚举
 */
enum class EXPORT_LUA_ENUM() ObjectType {
    UNKNOWN,
    PLAYER,
    ENEMY,
    ITEM,
    PROJECTILE,
    ENVIRONMENT
};

/**
 * @brief 游戏状态枚举
 */
enum class EXPORT_LUA_ENUM() GameState {
    MENU,
    LOADING,
    PLAYING,
    PAUSED,
    GAME_OVER
};

/**
 * @brief 游戏配置常量
 */
EXPORT_LUA_CONSTANT()
static const int MAX_ENTITIES = 1000;

EXPORT_LUA_CONSTANT()
static const double WORLD_WIDTH = 1920.0;

EXPORT_LUA_CONSTANT()
static const double WORLD_HEIGHT = 1080.0;

EXPORT_LUA_CONSTANT()
static const std::string ENGINE_VERSION = "1.0.0";

/**
 * @brief 时间管理类
 */
class EXPORT_LUA_CLASS() Time {
public:
    Time();
    
    // 时间属性
    double getDeltaTime() const;
    double getTotalTime() const;
    int getFrameCount() const;
    double getFPS() const;
    
    // 时间控制
    void update();
    void pause();
    void resume();
    void setTimeScale(double scale);
    double getTimeScale() const;
    
    // 静态时间函数
    static double getCurrentTime();
    static void sleep(int milliseconds);

private:
    double delta_time_;
    double total_time_;
    double time_scale_;
    int frame_count_;
    double fps_;
    bool paused_;
    std::chrono::high_resolution_clock::time_point last_frame_time_;
};

/**
 * @brief 2D向量类（用于位置、速度等）
 */
class EXPORT_LUA_CLASS() Vector2 {
public:
    Vector2();
    Vector2(double x, double y);
    Vector2(const Vector2& other);
    
    // 属性访问
    double getX() const;
    void setX(double x);
    double getY() const;
    void setY(double y);
    
    // 向量运算
    EXPORT_LUA_OPERATOR(+)
    Vector2 operator+(const Vector2& other) const;
    
    EXPORT_LUA_OPERATOR(-)
    Vector2 operator-(const Vector2& other) const;
    
    EXPORT_LUA_OPERATOR(*)
    Vector2 operator*(double scalar) const;
    
    EXPORT_LUA_OPERATOR(/)
    Vector2 operator/(double scalar) const;
    
    EXPORT_LUA_OPERATOR(==)
    bool operator==(const Vector2& other) const;
    
    // 向量数学
    double length() const;
    double lengthSquared() const;
    Vector2 normalized() const;
    double distance(const Vector2& other) const;
    double dot(const Vector2& other) const;
    
    // 静态方法
    static Vector2 zero();
    static Vector2 one();
    static Vector2 up();
    static Vector2 down();
    static Vector2 left();
    static Vector2 right();

private:
    double x_, y_;
};

/**
 * @brief 基础游戏对象
 */
class EXPORT_LUA_CLASS() GameObject {
public:
    GameObject();
    GameObject(const std::string& name, ObjectType type);
    virtual ~GameObject();
    
    // 基本属性
    int getId() const;
    std::string getName() const;
    void setName(const std::string& name);
    
    ObjectType getType() const;
    void setType(ObjectType type);
    
    bool isActive() const;
    void setActive(bool active);
    
    // 变换属性
    Vector2 getPosition() const;
    void setPosition(const Vector2& position);
    
    double getRotation() const;
    void setRotation(double rotation);
    
    Vector2 getScale() const;
    void setScale(const Vector2& scale);
    
    // 游戏逻辑
    virtual void start();
    virtual void update(double deltaTime);
    virtual void render();
    virtual void destroy();
    
    // 组件系统（简化版）
    void addTag(const std::string& tag);
    void removeTag(const std::string& tag);
    bool hasTag(const std::string& tag) const;
    std::vector<std::string> getTags() const;
    
    // 静态方法
    static int getNextId();
    static std::shared_ptr<GameObject> create(const std::string& name, ObjectType type);

protected:
    int id_;
    std::string name_;
    ObjectType type_;
    bool active_;
    
    Vector2 position_;
    double rotation_;
    Vector2 scale_;
    
    std::vector<std::string> tags_;
    
    static int next_id_;
};

/**
 * @brief 角色类 - 可移动的游戏对象
 */
class EXPORT_LUA_CLASS() Character : public GameObject {
public:
    Character();
    Character(const std::string& name, double health, double speed);
    ~Character() override;
    
    // 角色属性
    double getHealth() const;
    void setHealth(double health);
    double getMaxHealth() const;
    void setMaxHealth(double max_health);
    
    double getSpeed() const;
    void setSpeed(double speed);
    
    Vector2 getVelocity() const;
    void setVelocity(const Vector2& velocity);
    
    // 角色状态
    bool isAlive() const;
    bool isMoving() const;
    
    // 角色行为
    void move(const Vector2& direction);
    void moveTo(const Vector2& target);
    void stop();
    
    void takeDamage(double damage);
    void heal(double amount);
    
    // 重写基类方法
    void update(double deltaTime) override;

protected:
    double health_;
    double max_health_;
    double speed_;
    Vector2 velocity_;
    Vector2 target_position_;
    bool moving_to_target_;
};

/**
 * @brief 玩家类
 */
class EXPORT_LUA_CLASS() Player : public Character {
public:
    Player();
    Player(const std::string& name);
    ~Player() override;
    
    // 玩家属性
    int getLevel() const;
    void setLevel(int level);
    
    int getExperience() const;
    void addExperience(int exp);
    
    int getScore() const;
    void addScore(int points);
    
    int getLives() const;
    void setLives(int lives);
    
    // 玩家能力
    std::vector<std::string> getAbilities() const;
    void addAbility(const std::string& ability);
    bool hasAbility(const std::string& ability) const;
    
    // 玩家输入处理
    void handleInput(const std::string& action, bool pressed);
    
    // 重写方法
    void start() override;
    void update(double deltaTime) override;

private:
    int level_;
    int experience_;
    int score_;
    int lives_;
    std::vector<std::string> abilities_;
    std::unordered_map<std::string, bool> input_state_;
};

/**
 * @brief 敌人类
 */
class EXPORT_LUA_CLASS() Enemy : public Character {
public:
    Enemy();
    Enemy(const std::string& name, double health, double damage);
    ~Enemy() override;
    
    // 敌人属性
    double getDamage() const;
    void setDamage(double damage);
    
    double getAttackRange() const;
    void setAttackRange(double range);
    
    double getDetectionRange() const;
    void setDetectionRange(double range);
    
    // 敌人AI
    std::shared_ptr<Player> getTarget() const;
    void setTarget(std::shared_ptr<Player> target);
    
    void patrol(const std::vector<Vector2>& waypoints);
    void chase(std::shared_ptr<GameObject> target);
    void attack(std::shared_ptr<GameObject> target);
    
    // 重写方法
    void update(double deltaTime) override;

private:
    double damage_;
    double attack_range_;
    double detection_range_;
    std::shared_ptr<Player> target_;
    std::vector<Vector2> patrol_points_;
    int current_patrol_index_;
    double last_attack_time_;
};

} // namespace core

namespace systems {

/**
 * @brief 游戏世界管理器
 */
class EXPORT_LUA_SINGLETON() World {
public:
    static World& getInstance();
    
    // 禁用拷贝
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    
    // 游戏对象管理
    void addGameObject(std::shared_ptr<core::GameObject> object);
    void removeGameObject(int id);
    std::shared_ptr<core::GameObject> findGameObject(int id) const;
    std::shared_ptr<core::GameObject> findGameObjectByName(const std::string& name) const;
    
    std::vector<std::shared_ptr<core::GameObject>> getAllGameObjects() const;
    std::vector<std::shared_ptr<core::GameObject>> findGameObjectsByType(core::ObjectType type) const;
    std::vector<std::shared_ptr<core::GameObject>> findGameObjectsByTag(const std::string& tag) const;
    
    // 世界更新
    void update(double deltaTime);
    void render();
    void clear();
    
    // 世界属性
    int getObjectCount() const;
    std::shared_ptr<core::Player> getPlayer() const;
    void setPlayer(std::shared_ptr<core::Player> player);

private:
    World();
    ~World();
    
    std::unordered_map<int, std::shared_ptr<core::GameObject>> objects_;
    std::shared_ptr<core::Player> player_;
};

/**
 * @brief 游戏管理器
 */
class EXPORT_LUA_SINGLETON() GameManager {
public:
    static GameManager& getInstance();
    
    // 禁用拷贝
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;
    
    // 游戏状态管理
    core::GameState getGameState() const;
    void setGameState(core::GameState state);
    
    bool isGameRunning() const;
    bool isGamePaused() const;
    
    // 游戏生命周期
    void initializeGame();
    void startGame();
    void pauseGame();
    void resumeGame();
    void stopGame();
    void restartGame();
    
    // 关卡管理
    void loadLevel(const std::string& levelName);
    void nextLevel();
    void restartLevel();
    std::string getCurrentLevel() const;
    
    // 游戏统计
    double getGameTime() const;
    int getCurrentLevelNumber() const;
    int getHighScore() const;
    void setHighScore(int score);
    
    // 游戏循环
    void update();

private:
    GameManager();
    ~GameManager();
    
    core::GameState game_state_;
    std::string current_level_;
    int current_level_number_;
    double game_time_;
    int high_score_;
    std::unique_ptr<core::Time> time_manager_;
};

/**
 * @brief 事件系统
 */
class EXPORT_LUA_CLASS() EventManager {
public:
    EventManager();
    ~EventManager();
    
    // 游戏事件回调
    EXPORT_LUA_CALLBACK()
    std::function<void()> OnGameStart;
    
    EXPORT_LUA_CALLBACK()
    std::function<void()> OnGameEnd;
    
    EXPORT_LUA_CALLBACK()
    std::function<void(core::GameState)> OnGameStateChanged;
    
    EXPORT_LUA_CALLBACK()
    std::function<void(std::shared_ptr<core::Player>)> OnPlayerSpawn;
    
    EXPORT_LUA_CALLBACK()
    std::function<void(std::shared_ptr<core::Player>)> OnPlayerDeath;
    
    EXPORT_LUA_CALLBACK()
    std::function<void(std::shared_ptr<core::Player>, int)> OnPlayerLevelUp;
    
    EXPORT_LUA_CALLBACK()
    std::function<void(std::shared_ptr<core::Enemy>)> OnEnemySpawn;
    
    EXPORT_LUA_CALLBACK()
    std::function<void(std::shared_ptr<core::Enemy>)> OnEnemyDeath;
    
    EXPORT_LUA_CALLBACK()
    std::function<void(const std::string&)> OnLevelComplete;
    
    // 事件触发方法
    void triggerGameStart();
    void triggerGameEnd();
    void triggerGameStateChanged(core::GameState newState);
    void triggerPlayerSpawn(std::shared_ptr<core::Player> player);
    void triggerPlayerDeath(std::shared_ptr<core::Player> player);
    void triggerPlayerLevelUp(std::shared_ptr<core::Player> player, int newLevel);
    void triggerEnemySpawn(std::shared_ptr<core::Enemy> enemy);
    void triggerEnemyDeath(std::shared_ptr<core::Enemy> enemy);
    void triggerLevelComplete(const std::string& levelName);

private:
    bool initialized_;
};

} // namespace systems

namespace utils {

/**
 * @brief 数学工具类
 */
class EXPORT_LUA_STATIC_CLASS() MathUtils {
public:
    // 基本数学函数
    static double clamp(double value, double min, double max);
    static double lerp(double a, double b, double t);
    static double smoothStep(double edge0, double edge1, double x);
    
    // 随机数生成
    static int randomInt(int min, int max);
    static double randomFloat(double min, double max);
    static bool randomBool();
    static core::Vector2 randomVector2(double min, double max);
    
    // 角度转换
    static double degreesToRadians(double degrees);
    static double radiansToDegrees(double radians);
    
    // 距离和几何
    static double distance(const core::Vector2& a, const core::Vector2& b);
    static double angle(const core::Vector2& from, const core::Vector2& to);
    static core::Vector2 rotateVector(const core::Vector2& vector, double angle);
    
    // 常量
    static const double PI;
    static const double TWO_PI;
    static const double HALF_PI;
    static const double DEG_TO_RAD;
    static const double RAD_TO_DEG;
};

/**
 * @brief 碰撞检测工具
 */
class EXPORT_LUA_STATIC_CLASS() CollisionUtils {
public:
    // 点碰撞检测
    static bool pointInCircle(const core::Vector2& point, const core::Vector2& center, double radius);
    static bool pointInRect(const core::Vector2& point, const core::Vector2& rectPos, const core::Vector2& rectSize);
    
    // 形状碰撞检测
    static bool circleToCircle(const core::Vector2& pos1, double radius1, const core::Vector2& pos2, double radius2);
    static bool rectToRect(const core::Vector2& pos1, const core::Vector2& size1, const core::Vector2& pos2, const core::Vector2& size2);
    static bool circleToRect(const core::Vector2& circlePos, double radius, const core::Vector2& rectPos, const core::Vector2& rectSize);
    
    // 游戏对象碰撞检测
    static bool checkCollision(std::shared_ptr<core::GameObject> obj1, std::shared_ptr<core::GameObject> obj2);
    static std::vector<std::shared_ptr<core::GameObject>> findCollisions(
        std::shared_ptr<core::GameObject> object, 
        const std::vector<std::shared_ptr<core::GameObject>>& candidates
    );
};

/**
 * @brief 资源管理器（简化版）
 */
class EXPORT_LUA_CLASS() ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();
    
    // 文本资源
    bool loadText(const std::string& name, const std::string& content);
    std::string getText(const std::string& name) const;
    bool hasText(const std::string& name) const;
    
    // 配置资源
    bool loadConfig(const std::string& name, const std::unordered_map<std::string, std::string>& config);
    std::string getConfigValue(const std::string& configName, const std::string& key) const;
    std::unordered_map<std::string, std::string> getConfig(const std::string& name) const;
    
    // 预制体（游戏对象模板）
    bool loadPrefab(const std::string& name, std::shared_ptr<core::GameObject> prefab);
    std::shared_ptr<core::GameObject> createFromPrefab(const std::string& name) const;
    
    // 资源管理
    void unload(const std::string& name);
    void unloadAll();
    std::vector<std::string> getLoadedResources() const;
    
    // 内存统计
    int getResourceCount() const;

private:
    std::unordered_map<std::string, std::string> texts_;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> configs_;
    std::unordered_map<std::string, std::shared_ptr<core::GameObject>> prefabs_;
};

} // namespace utils

} // namespace engine

// STL容器导出
EXPORT_LUA_VECTOR(std::string)
EXPORT_LUA_VECTOR(std::shared_ptr<engine::core::GameObject>)
EXPORT_LUA_VECTOR(std::shared_ptr<engine::core::Player>)
EXPORT_LUA_VECTOR(std::shared_ptr<engine::core::Enemy>)
EXPORT_LUA_VECTOR(engine::core::Vector2)
EXPORT_LUA_UNORDERED_MAP(std::string, std::string)