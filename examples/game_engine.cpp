/**
 * @file game_engine.cpp
 * @brief game_engine.h 的完整实现
 */

#include "game_engine.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <cmath>
#include <thread>

namespace engine {
namespace core {

// 静态变量初始化
int GameObject::next_id_ = 1;

// ================================
// Time 类实现
// ================================

Time::Time() 
    : delta_time_(0.0), total_time_(0.0), time_scale_(1.0), 
      frame_count_(0), fps_(0.0), paused_(false) {
    last_frame_time_ = std::chrono::high_resolution_clock::now();
}

double Time::getDeltaTime() const {
    return delta_time_ * time_scale_;
}

double Time::getTotalTime() const {
    return total_time_;
}

int Time::getFrameCount() const {
    return frame_count_;
}

double Time::getFPS() const {
    return fps_;
}

void Time::update() {
    if (paused_) return;
    
    auto current_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_frame_time_);
    delta_time_ = duration.count() / 1000000.0; // 转换为秒
    
    total_time_ += delta_time_;
    frame_count_++;
    
    // 计算FPS（每秒更新一次）
    static double fps_timer = 0.0;
    static int fps_frame_count = 0;
    fps_timer += delta_time_;
    fps_frame_count++;
    
    if (fps_timer >= 1.0) {
        fps_ = fps_frame_count / fps_timer;
        fps_timer = 0.0;
        fps_frame_count = 0;
    }
    
    last_frame_time_ = current_time;
}

void Time::pause() {
    paused_ = true;
}

void Time::resume() {
    paused_ = false;
    last_frame_time_ = std::chrono::high_resolution_clock::now();
}

void Time::setTimeScale(double scale) {
    time_scale_ = std::max(0.0, scale);
}

double Time::getTimeScale() const {
    return time_scale_;
}

double Time::getCurrentTime() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count() / 1000000.0;
}

void Time::sleep(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

// ================================
// Vector2 类实现
// ================================

Vector2::Vector2() : x_(0.0), y_(0.0) {
}

Vector2::Vector2(double x, double y) : x_(x), y_(y) {
}

Vector2::Vector2(const Vector2& other) : x_(other.x_), y_(other.y_) {
}

double Vector2::getX() const {
    return x_;
}

void Vector2::setX(double x) {
    x_ = x;
}

double Vector2::getY() const {
    return y_;
}

void Vector2::setY(double y) {
    y_ = y;
}

Vector2 Vector2::operator+(const Vector2& other) const {
    return Vector2(x_ + other.x_, y_ + other.y_);
}

Vector2 Vector2::operator-(const Vector2& other) const {
    return Vector2(x_ - other.x_, y_ - other.y_);
}

Vector2 Vector2::operator*(double scalar) const {
    return Vector2(x_ * scalar, y_ * scalar);
}

Vector2 Vector2::operator/(double scalar) const {
    if (std::abs(scalar) > 1e-9) {
        return Vector2(x_ / scalar, y_ / scalar);
    }
    return *this;
}

bool Vector2::operator==(const Vector2& other) const {
    return std::abs(x_ - other.x_) < 1e-9 && std::abs(y_ - other.y_) < 1e-9;
}

double Vector2::length() const {
    return std::sqrt(x_ * x_ + y_ * y_);
}

double Vector2::lengthSquared() const {
    return x_ * x_ + y_ * y_;
}

Vector2 Vector2::normalized() const {
    double len = length();
    if (len > 1e-9) {
        return Vector2(x_ / len, y_ / len);
    }
    return Vector2(0, 0);
}

double Vector2::distance(const Vector2& other) const {
    return (*this - other).length();
}

double Vector2::dot(const Vector2& other) const {
    return x_ * other.x_ + y_ * other.y_;
}

Vector2 Vector2::zero() {
    return Vector2(0, 0);
}

Vector2 Vector2::one() {
    return Vector2(1, 1);
}

Vector2 Vector2::up() {
    return Vector2(0, 1);
}

Vector2 Vector2::down() {
    return Vector2(0, -1);
}

Vector2 Vector2::left() {
    return Vector2(-1, 0);
}

Vector2 Vector2::right() {
    return Vector2(1, 0);
}

// ================================
// GameObject 类实现
// ================================

GameObject::GameObject() 
    : id_(next_id_++), name_("GameObject_" + std::to_string(id_)), 
      type_(ObjectType::UNKNOWN), active_(true),
      position_(Vector2::zero()), rotation_(0.0), scale_(Vector2::one()) {
}

GameObject::GameObject(const std::string& name, ObjectType type)
    : id_(next_id_++), name_(name), type_(type), active_(true),
      position_(Vector2::zero()), rotation_(0.0), scale_(Vector2::one()) {
}

GameObject::~GameObject() {
}

int GameObject::getId() const {
    return id_;
}

std::string GameObject::getName() const {
    return name_;
}

void GameObject::setName(const std::string& name) {
    name_ = name;
}

ObjectType GameObject::getType() const {
    return type_;
}

void GameObject::setType(ObjectType type) {
    type_ = type;
}

bool GameObject::isActive() const {
    return active_;
}

void GameObject::setActive(bool active) {
    active_ = active;
}

Vector2 GameObject::getPosition() const {
    return position_;
}

void GameObject::setPosition(const Vector2& position) {
    position_ = position;
}

double GameObject::getRotation() const {
    return rotation_;
}

void GameObject::setRotation(double rotation) {
    rotation_ = rotation;
}

Vector2 GameObject::getScale() const {
    return scale_;
}

void GameObject::setScale(const Vector2& scale) {
    scale_ = scale;
}

void GameObject::start() {
    // 基类默认实现
}

void GameObject::update(double deltaTime) {
    // 基类默认实现
}

void GameObject::render() {
    // 基类默认实现
}

void GameObject::destroy() {
    active_ = false;
}

void GameObject::addTag(const std::string& tag) {
    if (std::find(tags_.begin(), tags_.end(), tag) == tags_.end()) {
        tags_.push_back(tag);
    }
}

void GameObject::removeTag(const std::string& tag) {
    auto it = std::find(tags_.begin(), tags_.end(), tag);
    if (it != tags_.end()) {
        tags_.erase(it);
    }
}

bool GameObject::hasTag(const std::string& tag) const {
    return std::find(tags_.begin(), tags_.end(), tag) != tags_.end();
}

std::vector<std::string> GameObject::getTags() const {
    return tags_;
}

int GameObject::getNextId() {
    return next_id_;
}

std::shared_ptr<GameObject> GameObject::create(const std::string& name, ObjectType type) {
    return std::make_shared<GameObject>(name, type);
}

// ================================
// Character 类实现
// ================================

Character::Character() 
    : GameObject(), health_(100.0), max_health_(100.0), speed_(100.0),
      velocity_(Vector2::zero()), target_position_(Vector2::zero()), moving_to_target_(false) {
    setType(ObjectType::PLAYER);
}

Character::Character(const std::string& name, double health, double speed)
    : GameObject(name, ObjectType::PLAYER), health_(health), max_health_(health), speed_(speed),
      velocity_(Vector2::zero()), target_position_(Vector2::zero()), moving_to_target_(false) {
}

Character::~Character() {
}

double Character::getHealth() const {
    return health_;
}

void Character::setHealth(double health) {
    health_ = std::max(0.0, std::min(max_health_, health));
}

double Character::getMaxHealth() const {
    return max_health_;
}

void Character::setMaxHealth(double max_health) {
    max_health_ = std::max(1.0, max_health);
    health_ = std::min(health_, max_health_);
}

double Character::getSpeed() const {
    return speed_;
}

void Character::setSpeed(double speed) {
    speed_ = std::max(0.0, speed);
}

Vector2 Character::getVelocity() const {
    return velocity_;
}

void Character::setVelocity(const Vector2& velocity) {
    velocity_ = velocity;
}

bool Character::isAlive() const {
    return health_ > 0.0;
}

bool Character::isMoving() const {
    return velocity_.lengthSquared() > 1e-6 || moving_to_target_;
}

void Character::move(const Vector2& direction) {
    velocity_ = direction.normalized() * speed_;
    moving_to_target_ = false;
}

void Character::moveTo(const Vector2& target) {
    target_position_ = target;
    moving_to_target_ = true;
}

void Character::stop() {
    velocity_ = Vector2::zero();
    moving_to_target_ = false;
}

void Character::takeDamage(double damage) {
    if (damage > 0.0) {
        health_ = std::max(0.0, health_ - damage);
        if (health_ <= 0.0) {
            // 角色死亡逻辑
            std::cout << getName() << " has died!" << std::endl;
        }
    }
}

void Character::heal(double amount) {
    if (amount > 0.0) {
        health_ = std::min(max_health_, health_ + amount);
    }
}

void Character::update(double deltaTime) {
    GameObject::update(deltaTime);
    
    if (!isAlive()) return;
    
    // 移动到目标位置
    if (moving_to_target_) {
        Vector2 direction = target_position_ - position_;
        double distance = direction.length();
        
        if (distance < 1.0) {
            // 到达目标
            position_ = target_position_;
            velocity_ = Vector2::zero();
            moving_to_target_ = false;
        } else {
            // 继续移动
            velocity_ = direction.normalized() * speed_;
        }
    }
    
    // 应用速度
    if (velocity_.lengthSquared() > 1e-6) {
        position_ = position_ + velocity_ * deltaTime;
    }
}

// ================================
// Player 类实现
// ================================

Player::Player() 
    : Character("Player", 100.0, 150.0), level_(1), experience_(0), score_(0), lives_(3) {
    setType(ObjectType::PLAYER);
    abilities_.push_back("Basic Movement");
}

Player::Player(const std::string& name)
    : Character(name, 100.0, 150.0), level_(1), experience_(0), score_(0), lives_(3) {
    setType(ObjectType::PLAYER);
    abilities_.push_back("Basic Movement");
}

Player::~Player() {
}

int Player::getLevel() const {
    return level_;
}

void Player::setLevel(int level) {
    level_ = std::max(1, level);
}

int Player::getExperience() const {
    return experience_;
}

void Player::addExperience(int exp) {
    if (exp > 0) {
        experience_ += exp;
        
        // 检查升级（每100经验升一级）
        int required_exp = level_ * 100;
        if (experience_ >= required_exp) {
            level_++;
            experience_ -= required_exp;
            std::cout << getName() << " leveled up to level " << level_ << "!" << std::endl;
        }
    }
}

int Player::getScore() const {
    return score_;
}

void Player::addScore(int points) {
    if (points > 0) {
        score_ += points;
    }
}

int Player::getLives() const {
    return lives_;
}

void Player::setLives(int lives) {
    lives_ = std::max(0, lives);
}

std::vector<std::string> Player::getAbilities() const {
    return abilities_;
}

void Player::addAbility(const std::string& ability) {
    if (std::find(abilities_.begin(), abilities_.end(), ability) == abilities_.end()) {
        abilities_.push_back(ability);
        std::cout << getName() << " learned new ability: " << ability << std::endl;
    }
}

bool Player::hasAbility(const std::string& ability) const {
    return std::find(abilities_.begin(), abilities_.end(), ability) != abilities_.end();
}

void Player::handleInput(const std::string& action, bool pressed) {
    input_state_[action] = pressed;
    
    // 处理移动输入
    Vector2 move_direction = Vector2::zero();
    
    if (input_state_["move_up"]) move_direction = move_direction + Vector2::up();
    if (input_state_["move_down"]) move_direction = move_direction + Vector2::down();
    if (input_state_["move_left"]) move_direction = move_direction + Vector2::left();
    if (input_state_["move_right"]) move_direction = move_direction + Vector2::right();
    
    if (move_direction.lengthSquared() > 0.1) {
        move(move_direction);
    } else {
        stop();
    }
}

void Player::start() {
    Character::start();
    std::cout << "Player " << getName() << " has spawned!" << std::endl;
}

void Player::update(double deltaTime) {
    Character::update(deltaTime);
    
    // 玩家特有的更新逻辑
    // 例如：技能冷却、buff效果等
}

// ================================
// Enemy 类实现
// ================================

Enemy::Enemy() 
    : Character("Enemy", 50.0, 80.0), damage_(10.0), attack_range_(30.0), 
      detection_range_(100.0), current_patrol_index_(0), last_attack_time_(0.0) {
    setType(ObjectType::ENEMY);
}

Enemy::Enemy(const std::string& name, double health, double damage)
    : Character(name, health, 80.0), damage_(damage), attack_range_(30.0),
      detection_range_(100.0), current_patrol_index_(0), last_attack_time_(0.0) {
    setType(ObjectType::ENEMY);
}

Enemy::~Enemy() {
}

double Enemy::getDamage() const {
    return damage_;
}

void Enemy::setDamage(double damage) {
    damage_ = std::max(0.0, damage);
}

double Enemy::getAttackRange() const {
    return attack_range_;
}

void Enemy::setAttackRange(double range) {
    attack_range_ = std::max(0.0, range);
}

double Enemy::getDetectionRange() const {
    return detection_range_;
}

void Enemy::setDetectionRange(double range) {
    detection_range_ = std::max(0.0, range);
}

std::shared_ptr<Player> Enemy::getTarget() const {
    return target_;
}

void Enemy::setTarget(std::shared_ptr<Player> target) {
    target_ = target;
}

void Enemy::patrol(const std::vector<Vector2>& waypoints) {
    patrol_points_ = waypoints;
    current_patrol_index_ = 0;
    
    if (!patrol_points_.empty()) {
        moveTo(patrol_points_[current_patrol_index_]);
    }
}

void Enemy::chase(std::shared_ptr<GameObject> target) {
    if (target && target->isActive()) {
        moveTo(target->getPosition());
    }
}

void Enemy::attack(std::shared_ptr<GameObject> target) {
    double current_time = Time::getCurrentTime();
    
    if (current_time - last_attack_time_ >= 1.0) { // 1秒攻击间隔
        if (target && target->isActive()) {
            double dist = position_.distance(target->getPosition());
            if (dist <= attack_range_) {
                // 执行攻击
                auto character = std::dynamic_pointer_cast<Character>(target);
                if (character) {
                    character->takeDamage(damage_);
                    std::cout << getName() << " attacks " << target->getName() 
                              << " for " << damage_ << " damage!" << std::endl;
                }
                last_attack_time_ = current_time;
            }
        }
    }
}

void Enemy::update(double deltaTime) {
    Character::update(deltaTime);
    
    if (!isAlive()) return;
    
    // 简单AI逻辑
    if (target_ && target_->isActive() && target_->isAlive()) {
        double distance_to_target = position_.distance(target_->getPosition());
        
        if (distance_to_target <= detection_range_) {
            if (distance_to_target <= attack_range_) {
                // 在攻击范围内 - 停止移动并攻击
                stop();
                attack(target_);
            } else {
                // 在检测范围内但不在攻击范围 - 追击
                chase(target_);
            }
        } else {
            // 超出检测范围 - 巡逻
            if (!patrol_points_.empty() && !isMoving()) {
                current_patrol_index_ = (current_patrol_index_ + 1) % patrol_points_.size();
                moveTo(patrol_points_[current_patrol_index_]);
            }
        }
    }
}

} // namespace core

namespace systems {

// ================================
// World 类实现（单例）
// ================================

World::World() {
}

World::~World() {
    clear();
}

World& World::getInstance() {
    static World instance;
    return instance;
}

void World::addGameObject(std::shared_ptr<core::GameObject> object) {
    if (object) {
        objects_[object->getId()] = object;
        object->start();
        
        // 如果是玩家，设置为当前玩家
        auto player = std::dynamic_pointer_cast<core::Player>(object);
        if (player && !player_) {
            player_ = player;
        }
        
        std::cout << "Added " << object->getName() << " to world" << std::endl;
    }
}

void World::removeGameObject(int id) {
    auto it = objects_.find(id);
    if (it != objects_.end()) {
        it->second->destroy();
        objects_.erase(it);
    }
}

std::shared_ptr<core::GameObject> World::findGameObject(int id) const {
    auto it = objects_.find(id);
    return (it != objects_.end()) ? it->second : nullptr;
}

std::shared_ptr<core::GameObject> World::findGameObjectByName(const std::string& name) const {
    for (const auto& pair : objects_) {
        if (pair.second->getName() == name) {
            return pair.second;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<core::GameObject>> World::getAllGameObjects() const {
    std::vector<std::shared_ptr<core::GameObject>> result;
    for (const auto& pair : objects_) {
        if (pair.second->isActive()) {
            result.push_back(pair.second);
        }
    }
    return result;
}

std::vector<std::shared_ptr<core::GameObject>> World::findGameObjectsByType(core::ObjectType type) const {
    std::vector<std::shared_ptr<core::GameObject>> result;
    for (const auto& pair : objects_) {
        if (pair.second->isActive() && pair.second->getType() == type) {
            result.push_back(pair.second);
        }
    }
    return result;
}

std::vector<std::shared_ptr<core::GameObject>> World::findGameObjectsByTag(const std::string& tag) const {
    std::vector<std::shared_ptr<core::GameObject>> result;
    for (const auto& pair : objects_) {
        if (pair.second->isActive() && pair.second->hasTag(tag)) {
            result.push_back(pair.second);
        }
    }
    return result;
}

void World::update(double deltaTime) {
    // 更新所有活跃的游戏对象
    for (auto& pair : objects_) {
        if (pair.second->isActive()) {
            pair.second->update(deltaTime);
        }
    }
    
    // 移除已销毁的对象
    auto it = objects_.begin();
    while (it != objects_.end()) {
        if (!it->second->isActive()) {
            it = objects_.erase(it);
        } else {
            ++it;
        }
    }
}

void World::render() {
    for (const auto& pair : objects_) {
        if (pair.second->isActive()) {
            pair.second->render();
        }
    }
}

void World::clear() {
    objects_.clear();
    player_.reset();
}

int World::getObjectCount() const {
    return static_cast<int>(objects_.size());
}

std::shared_ptr<core::Player> World::getPlayer() const {
    return player_;
}

void World::setPlayer(std::shared_ptr<core::Player> player) {
    player_ = player;
}

// ================================
// GameManager 类实现（单例）
// ================================

GameManager::GameManager() 
    : game_state_(core::GameState::MENU), current_level_("Level1"), 
      current_level_number_(1), game_time_(0.0), high_score_(0) {
    time_manager_ = std::make_unique<core::Time>();
}

GameManager::~GameManager() {
}

GameManager& GameManager::getInstance() {
    static GameManager instance;
    return instance;
}

core::GameState GameManager::getGameState() const {
    return game_state_;
}

void GameManager::setGameState(core::GameState state) {
    if (game_state_ != state) {
        game_state_ = state;
        std::cout << "Game state changed to: " << static_cast<int>(state) << std::endl;
    }
}

bool GameManager::isGameRunning() const {
    return game_state_ == core::GameState::PLAYING;
}

bool GameManager::isGamePaused() const {
    return game_state_ == core::GameState::PAUSED;
}

void GameManager::initializeGame() {
    std::cout << "Initializing game..." << std::endl;
    setGameState(core::GameState::LOADING);
    
    // 初始化游戏系统
    World::getInstance().clear();
    game_time_ = 0.0;
    
    setGameState(core::GameState::MENU);
}

void GameManager::startGame() {
    std::cout << "Starting game..." << std::endl;
    setGameState(core::GameState::PLAYING);
    time_manager_->resume();
}

void GameManager::pauseGame() {
    if (isGameRunning()) {
        std::cout << "Pausing game..." << std::endl;
        setGameState(core::GameState::PAUSED);
        time_manager_->pause();
    }
}

void GameManager::resumeGame() {
    if (isGamePaused()) {
        std::cout << "Resuming game..." << std::endl;
        setGameState(core::GameState::PLAYING);
        time_manager_->resume();
    }
}

void GameManager::stopGame() {
    std::cout << "Stopping game..." << std::endl;
    setGameState(core::GameState::GAME_OVER);
    time_manager_->pause();
}

void GameManager::restartGame() {
    std::cout << "Restarting game..." << std::endl;
    initializeGame();
    startGame();
}

void GameManager::loadLevel(const std::string& levelName) {
    current_level_ = levelName;
    std::cout << "Loading level: " << levelName << std::endl;
    
    // 清理当前世界
    World::getInstance().clear();
    
    // 这里可以添加关卡加载逻辑
}

void GameManager::nextLevel() {
    current_level_number_++;
    loadLevel("Level" + std::to_string(current_level_number_));
}

void GameManager::restartLevel() {
    loadLevel(current_level_);
}

std::string GameManager::getCurrentLevel() const {
    return current_level_;
}

double GameManager::getGameTime() const {
    return game_time_;
}

int GameManager::getCurrentLevelNumber() const {
    return current_level_number_;
}

int GameManager::getHighScore() const {
    return high_score_;
}

void GameManager::setHighScore(int score) {
    high_score_ = std::max(high_score_, score);
}

void GameManager::update() {
    if (isGameRunning()) {
        time_manager_->update();
        game_time_ += time_manager_->getDeltaTime();
        
        // 更新游戏世界
        World::getInstance().update(time_manager_->getDeltaTime());
    }
}

// ================================
// EventManager 类实现
// ================================

EventManager::EventManager() : initialized_(true) {
    std::cout << "EventManager initialized" << std::endl;
}

EventManager::~EventManager() {
    std::cout << "EventManager destroyed" << std::endl;
}

void EventManager::triggerGameStart() {
    if (OnGameStart) {
        OnGameStart();
    }
}

void EventManager::triggerGameEnd() {
    if (OnGameEnd) {
        OnGameEnd();
    }
}

void EventManager::triggerGameStateChanged(core::GameState newState) {
    if (OnGameStateChanged) {
        OnGameStateChanged(newState);
    }
}

void EventManager::triggerPlayerSpawn(std::shared_ptr<core::Player> player) {
    if (OnPlayerSpawn && player) {
        OnPlayerSpawn(player);
    }
}

void EventManager::triggerPlayerDeath(std::shared_ptr<core::Player> player) {
    if (OnPlayerDeath && player) {
        OnPlayerDeath(player);
    }
}

void EventManager::triggerPlayerLevelUp(std::shared_ptr<core::Player> player, int newLevel) {
    if (OnPlayerLevelUp && player) {
        OnPlayerLevelUp(player, newLevel);
    }
}

void EventManager::triggerEnemySpawn(std::shared_ptr<core::Enemy> enemy) {
    if (OnEnemySpawn && enemy) {
        OnEnemySpawn(enemy);
    }
}

void EventManager::triggerEnemyDeath(std::shared_ptr<core::Enemy> enemy) {
    if (OnEnemyDeath && enemy) {
        OnEnemyDeath(enemy);
    }
}

void EventManager::triggerLevelComplete(const std::string& levelName) {
    if (OnLevelComplete) {
        OnLevelComplete(levelName);
    }
}

} // namespace systems

namespace utils {

// 常量初始化
const double MathUtils::PI = 3.141592653589793;
const double MathUtils::TWO_PI = 2.0 * PI;
const double MathUtils::HALF_PI = PI / 2.0;
const double MathUtils::DEG_TO_RAD = PI / 180.0;
const double MathUtils::RAD_TO_DEG = 180.0 / PI;

// ================================
// MathUtils 类实现（静态类）
// ================================

double MathUtils::clamp(double value, double min, double max) {
    return std::max(min, std::min(max, value));
}

double MathUtils::lerp(double a, double b, double t) {
    return a + t * (b - a);
}

double MathUtils::smoothStep(double edge0, double edge1, double x) {
    double t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

int MathUtils::randomInt(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

double MathUtils::randomFloat(double min, double max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

bool MathUtils::randomBool() {
    return randomInt(0, 1) == 1;
}

core::Vector2 MathUtils::randomVector2(double min, double max) {
    return core::Vector2(randomFloat(min, max), randomFloat(min, max));
}

double MathUtils::degreesToRadians(double degrees) {
    return degrees * DEG_TO_RAD;
}

double MathUtils::radiansToDegrees(double radians) {
    return radians * RAD_TO_DEG;
}

double MathUtils::distance(const core::Vector2& a, const core::Vector2& b) {
    return a.distance(b);
}

double MathUtils::angle(const core::Vector2& from, const core::Vector2& to) {
    core::Vector2 direction = to - from;
    return std::atan2(direction.getY(), direction.getX());
}

core::Vector2 MathUtils::rotateVector(const core::Vector2& vector, double angle) {
    double cos_a = std::cos(angle);
    double sin_a = std::sin(angle);
    return core::Vector2(
        vector.getX() * cos_a - vector.getY() * sin_a,
        vector.getX() * sin_a + vector.getY() * cos_a
    );
}

// ================================
// CollisionUtils 类实现（静态类）
// ================================

bool CollisionUtils::pointInCircle(const core::Vector2& point, const core::Vector2& center, double radius) {
    return point.distance(center) <= radius;
}

bool CollisionUtils::pointInRect(const core::Vector2& point, const core::Vector2& rectPos, const core::Vector2& rectSize) {
    return point.getX() >= rectPos.getX() && point.getX() <= rectPos.getX() + rectSize.getX() &&
           point.getY() >= rectPos.getY() && point.getY() <= rectPos.getY() + rectSize.getY();
}

bool CollisionUtils::circleToCircle(const core::Vector2& pos1, double radius1, const core::Vector2& pos2, double radius2) {
    return pos1.distance(pos2) <= (radius1 + radius2);
}

bool CollisionUtils::rectToRect(const core::Vector2& pos1, const core::Vector2& size1, const core::Vector2& pos2, const core::Vector2& size2) {
    return !(pos1.getX() + size1.getX() < pos2.getX() || 
             pos2.getX() + size2.getX() < pos1.getX() ||
             pos1.getY() + size1.getY() < pos2.getY() || 
             pos2.getY() + size2.getY() < pos1.getY());
}

bool CollisionUtils::circleToRect(const core::Vector2& circlePos, double radius, const core::Vector2& rectPos, const core::Vector2& rectSize) {
    // 找到矩形上最接近圆心的点
    double closestX = MathUtils::clamp(circlePos.getX(), rectPos.getX(), rectPos.getX() + rectSize.getX());
    double closestY = MathUtils::clamp(circlePos.getY(), rectPos.getY(), rectPos.getY() + rectSize.getY());
    
    core::Vector2 closest(closestX, closestY);
    return circlePos.distance(closest) <= radius;
}

bool CollisionUtils::checkCollision(std::shared_ptr<core::GameObject> obj1, std::shared_ptr<core::GameObject> obj2) {
    if (!obj1 || !obj2 || !obj1->isActive() || !obj2->isActive()) {
        return false;
    }
    
    // 简单的圆形碰撞检测（使用scale.x作为半径）
    return circleToCircle(obj1->getPosition(), obj1->getScale().getX(), 
                         obj2->getPosition(), obj2->getScale().getX());
}

std::vector<std::shared_ptr<core::GameObject>> CollisionUtils::findCollisions(
    std::shared_ptr<core::GameObject> object, 
    const std::vector<std::shared_ptr<core::GameObject>>& candidates) {
    
    std::vector<std::shared_ptr<core::GameObject>> collisions;
    
    if (!object || !object->isActive()) {
        return collisions;
    }
    
    for (const auto& candidate : candidates) {
        if (candidate != object && checkCollision(object, candidate)) {
            collisions.push_back(candidate);
        }
    }
    
    return collisions;
}

// ================================
// ResourceManager 类实现
// ================================

ResourceManager::ResourceManager() {
    std::cout << "ResourceManager initialized" << std::endl;
}

ResourceManager::~ResourceManager() {
    unloadAll();
    std::cout << "ResourceManager destroyed" << std::endl;
}

bool ResourceManager::loadText(const std::string& name, const std::string& content) {
    texts_[name] = content;
    std::cout << "Loaded text resource: " << name << std::endl;
    return true;
}

std::string ResourceManager::getText(const std::string& name) const {
    auto it = texts_.find(name);
    return (it != texts_.end()) ? it->second : "";
}

bool ResourceManager::hasText(const std::string& name) const {
    return texts_.find(name) != texts_.end();
}

bool ResourceManager::loadConfig(const std::string& name, const std::unordered_map<std::string, std::string>& config) {
    configs_[name] = config;
    std::cout << "Loaded config resource: " << name << std::endl;
    return true;
}

std::string ResourceManager::getConfigValue(const std::string& configName, const std::string& key) const {
    auto configIt = configs_.find(configName);
    if (configIt != configs_.end()) {
        auto valueIt = configIt->second.find(key);
        if (valueIt != configIt->second.end()) {
            return valueIt->second;
        }
    }
    return "";
}

std::unordered_map<std::string, std::string> ResourceManager::getConfig(const std::string& name) const {
    auto it = configs_.find(name);
    return (it != configs_.end()) ? it->second : std::unordered_map<std::string, std::string>();
}

bool ResourceManager::loadPrefab(const std::string& name, std::shared_ptr<core::GameObject> prefab) {
    if (prefab) {
        prefabs_[name] = prefab;
        std::cout << "Loaded prefab resource: " << name << std::endl;
        return true;
    }
    return false;
}

std::shared_ptr<core::GameObject> ResourceManager::createFromPrefab(const std::string& name) const {
    auto it = prefabs_.find(name);
    if (it != prefabs_.end()) {
        // 创建预制体的副本
        auto original = it->second;
        auto copy = std::make_shared<core::GameObject>(original->getName(), original->getType());
        copy->setPosition(original->getPosition());
        copy->setRotation(original->getRotation());
        copy->setScale(original->getScale());
        copy->setActive(original->isActive());
        
        // 复制标签
        for (const auto& tag : original->getTags()) {
            copy->addTag(tag);
        }
        
        return copy;
    }
    return nullptr;
}

void ResourceManager::unload(const std::string& name) {
    texts_.erase(name);
    configs_.erase(name);
    prefabs_.erase(name);
}

void ResourceManager::unloadAll() {
    texts_.clear();
    configs_.clear();
    prefabs_.clear();
    std::cout << "All resources unloaded" << std::endl;
}

std::vector<std::string> ResourceManager::getLoadedResources() const {
    std::vector<std::string> resources;
    
    for (const auto& pair : texts_) {
        resources.push_back("text:" + pair.first);
    }
    for (const auto& pair : configs_) {
        resources.push_back("config:" + pair.first);
    }
    for (const auto& pair : prefabs_) {
        resources.push_back("prefab:" + pair.first);
    }
    
    return resources;
}

int ResourceManager::getResourceCount() const {
    return static_cast<int>(texts_.size() + configs_.size() + prefabs_.size());
}

} // namespace utils

} // namespace engine