/**
 * @file comprehensive_test.cpp
 * @brief comprehensive_test.h 的完整实现
 * 
 * 提供所有声明函数和类的具体实现，确保代码可以正常编译和运行
 */

#include "comprehensive_test.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include <cmath>

// ================================
// 全局函数实现
// ================================

namespace game {
namespace core {

// 静态变量初始化
int Entity::next_id_ = 1;
const double MathUtils::PI = 3.141592653589793;
const double MathUtils::E = 2.718281828459045;
const double MathUtils::EPSILON = 1e-9;

// 全局函数实现
double calculateDistance(double x1, double y1, double x2, double y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

std::string formatMessage(const std::string& msg, int level) {
    std::string prefix;
    switch (level) {
        case 0: prefix = "[DEBUG] "; break;
        case 1: prefix = "[INFO] "; break;
        case 2: prefix = "[WARNING] "; break;
        case 3: prefix = "[ERROR] "; break;
        default: prefix = "[UNKNOWN] "; break;
    }
    return prefix + msg;
}

bool validateInput(const std::string& input) {
    return !input.empty() && input.length() <= 255;
}

// ================================
// Entity 类实现
// ================================

Entity::Entity() : id_(next_id_++), name_("Entity_" + std::to_string(id_)) {
}

Entity::Entity(int id, const std::string& name) : id_(id), name_(name) {
    if (id >= next_id_) {
        next_id_ = id + 1;
    }
}

Entity::~Entity() {
}

int Entity::getId() const {
    return id_;
}

void Entity::setId(int id) {
    id_ = id;
}

std::string Entity::getName() const {
    return name_;
}

void Entity::setName(const std::string& name) {
    name_ = name;
}

void Entity::update(double deltaTime) {
    // 基类默认实现 - 什么都不做
}

std::string Entity::toString() const {
    return "Entity[id=" + std::to_string(id_) + ", name=" + name_ + "]";
}

int Entity::getNextId() {
    return next_id_;
}

void Entity::resetIdCounter() {
    next_id_ = 1;
}

// ================================
// Player 类实现
// ================================

Player::Player() 
    : Entity(), level_(1), health_(100.0), mana_(50.0) {
    skills_.push_back("Basic Attack");
    inventory_["Health Potion"] = 3;
}

Player::Player(int id, const std::string& name, int level)
    : Entity(id, name), level_(level), health_(100.0), mana_(50.0) {
    skills_.push_back("Basic Attack");
    inventory_["Health Potion"] = 3;
}

Player::~Player() {
}

void Player::update(double deltaTime) {
    // 玩家更新逻辑 - 缓慢恢复魔法值
    if (mana_ < 100.0) {
        mana_ = std::min(100.0, mana_ + deltaTime * 10.0);
    }
}

std::string Player::toString() const {
    return "Player[id=" + std::to_string(getId()) + 
           ", name=" + getName() + 
           ", level=" + std::to_string(level_) + 
           ", hp=" + std::to_string(health_) + "]";
}

int Player::getLevel() const {
    return level_;
}

void Player::setLevel(int level) {
    level_ = std::max(1, level);
}

double Player::getHealth() const {
    return health_;
}

void Player::setHealth(double health) {
    health_ = std::max(0.0, std::min(100.0, health));
}

double Player::getMana() const {
    return mana_;
}

void Player::setMana(double mana) {
    mana_ = std::max(0.0, std::min(100.0, mana));
}

std::shared_ptr<Entity> Player::getTarget() const {
    return target_;
}

void Player::setTarget(std::shared_ptr<Entity> target) {
    target_ = target;
}

std::vector<std::string> Player::getSkills() const {
    return skills_;
}

void Player::addSkill(const std::string& skill) {
    if (std::find(skills_.begin(), skills_.end(), skill) == skills_.end()) {
        skills_.push_back(skill);
    }
}

std::map<std::string, int> Player::getInventory() const {
    return inventory_;
}

void Player::addItem(const std::string& item, int count) {
    inventory_[item] += count;
}

Player& Player::operator+=(int experience) {
    // 每100经验升一级
    if (experience > 0) {
        level_ += experience / 100;
    }
    return *this;
}

bool Player::operator==(const Player& other) const {
    return getId() == other.getId();
}

bool Player::operator<(const Player& other) const {
    return level_ < other.level_;
}

// ================================
// GameManager 类实现（单例）
// ================================

GameManager::GameManager() : game_running_(false), game_time_(0.0) {
}

GameManager::~GameManager() {
}

GameManager& GameManager::getInstance() {
    static GameManager instance;
    return instance;
}

void GameManager::startGame() {
    game_running_ = true;
    game_time_ = 0.0;
    std::cout << "Game started!" << std::endl;
}

void GameManager::stopGame() {
    game_running_ = false;
    std::cout << "Game stopped! Total time: " << game_time_ << " seconds" << std::endl;
}

bool GameManager::isGameRunning() const {
    return game_running_;
}

void GameManager::addPlayer(std::shared_ptr<Player> player) {
    if (player) {
        players_[player->getId()] = player;
        std::cout << "Player " << player->getName() << " joined the game" << std::endl;
    }
}

void GameManager::removePlayer(int playerId) {
    auto it = players_.find(playerId);
    if (it != players_.end()) {
        std::cout << "Player " << it->second->getName() << " left the game" << std::endl;
        players_.erase(it);
    }
}

std::shared_ptr<Player> GameManager::getPlayer(int playerId) const {
    auto it = players_.find(playerId);
    return (it != players_.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<Player>> GameManager::getAllPlayers() const {
    std::vector<std::shared_ptr<Player>> result;
    for (const auto& pair : players_) {
        result.push_back(pair.second);
    }
    return result;
}

int GameManager::getPlayerCount() const {
    return static_cast<int>(players_.size());
}

double GameManager::getGameTime() const {
    return game_time_;
}

// ================================
// MathUtils 类实现（静态类）
// ================================

double MathUtils::clamp(double value, double min, double max) {
    return std::max(min, std::min(max, value));
}

double MathUtils::lerp(double a, double b, double t) {
    return a + t * (b - a);
}

int MathUtils::random(int min, int max) {
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

double MathUtils::dotProduct(double x1, double y1, double x2, double y2) {
    return x1 * x2 + y1 * y2;
}

double MathUtils::magnitude(double x, double y) {
    return std::sqrt(x * x + y * y);
}

void MathUtils::normalize(double& x, double& y) {
    double mag = magnitude(x, y);
    if (mag > EPSILON) {
        x /= mag;
        y /= mag;
    }
}

// ================================
// Component 类实现（抽象基类）
// ================================

bool Component::isActive() const {
    return active_;
}

void Component::setActive(bool active) {
    active_ = active;
}

// ================================
// TransformComponent 类实现
// ================================

TransformComponent::TransformComponent() : x_(0.0), y_(0.0), rotation_(0.0) {
}

TransformComponent::TransformComponent(double x, double y, double rotation)
    : x_(x), y_(y), rotation_(rotation) {
}

void TransformComponent::initialize() {
    std::cout << "TransformComponent initialized at (" << x_ << ", " << y_ << ")" << std::endl;
}

void TransformComponent::update(double deltaTime) {
    // 变换组件的更新逻辑（当前为空）
}

void TransformComponent::destroy() {
    std::cout << "TransformComponent destroyed" << std::endl;
}

std::string TransformComponent::getTypeName() const {
    return "TransformComponent";
}

double TransformComponent::getX() const {
    return x_;
}

void TransformComponent::setX(double x) {
    x_ = x;
}

double TransformComponent::getY() const {
    return y_;
}

void TransformComponent::setY(double y) {
    y_ = y;
}

double TransformComponent::getRotation() const {
    return rotation_;
}

void TransformComponent::setRotation(double rotation) {
    rotation_ = rotation;
}

void TransformComponent::translate(double dx, double dy) {
    x_ += dx;
    y_ += dy;
}

void TransformComponent::rotate(double angle) {
    rotation_ += angle;
}

} // namespace core

namespace events {

// ================================
// EventSystem 类实现
// ================================

EventSystem::EventSystem() : initialized_(true) {
    std::cout << "EventSystem initialized" << std::endl;
}

EventSystem::~EventSystem() {
    std::cout << "EventSystem destroyed" << std::endl;
}

void EventSystem::triggerGameStart() {
    if (OnGameStart) {
        OnGameStart();
    }
}

void EventSystem::triggerGameEnd() {
    if (OnGameEnd) {
        OnGameEnd();
    }
}

void EventSystem::triggerPlayerJoin(std::shared_ptr<core::Player> player) {
    if (OnPlayerJoin && player) {
        OnPlayerJoin(player);
    }
}

void EventSystem::triggerPlayerLeave(std::shared_ptr<core::Player> player) {
    if (OnPlayerLeave && player) {
        OnPlayerLeave(player);
    }
}

void EventSystem::triggerPlayerLevelUp(std::shared_ptr<core::Player> player, int oldLevel, int newLevel) {
    if (OnPlayerLevelUp && player) {
        OnPlayerLevelUp(player, oldLevel, newLevel);
    }
}

bool EventSystem::validateAction(const std::string& action, double value) {
    if (OnValidateAction) {
        return OnValidateAction(action, value);
    }
    return true; // 默认允许所有操作
}

} // namespace events

namespace containers {

// ================================
// ContainerUtils 类实现
// ================================

ContainerUtils::ContainerUtils() {
    // 初始化一些测试数据
    int_vector_ = {1, 2, 3, 4, 5};
    string_vector_ = {"hello", "world", "lua", "binding"};
    string_int_map_ = {{"one", 1}, {"two", 2}, {"three", 3}};
    string_double_map_ = {{"pi", 3.14159}, {"e", 2.71828}};
}

std::vector<int> ContainerUtils::getIntVector() const {
    return int_vector_;
}

std::vector<std::string> ContainerUtils::getStringVector() const {
    return string_vector_;
}

std::vector<std::shared_ptr<core::Player>> ContainerUtils::getPlayerVector() const {
    return player_vector_;
}

std::map<std::string, int> ContainerUtils::getStringIntMap() const {
    return string_int_map_;
}

std::map<int, std::shared_ptr<core::Player>> ContainerUtils::getPlayerMap() const {
    return player_map_;
}

std::unordered_map<std::string, double> ContainerUtils::getStringDoubleMap() const {
    return string_double_map_;
}

void ContainerUtils::processIntVector(const std::vector<int>& vec) {
    std::cout << "Processing int vector with " << vec.size() << " elements" << std::endl;
    int_vector_ = vec;
}

void ContainerUtils::processStringVector(const std::vector<std::string>& vec) {
    std::cout << "Processing string vector with " << vec.size() << " elements" << std::endl;
    string_vector_ = vec;
}

void ContainerUtils::processPlayerVector(const std::vector<std::shared_ptr<core::Player>>& vec) {
    std::cout << "Processing player vector with " << vec.size() << " players" << std::endl;
    player_vector_ = vec;
}

void ContainerUtils::processStringIntMap(const std::map<std::string, int>& map) {
    std::cout << "Processing string-int map with " << map.size() << " entries" << std::endl;
    string_int_map_ = map;
}

void ContainerUtils::processPlayerMap(const std::map<int, std::shared_ptr<core::Player>>& map) {
    std::cout << "Processing player map with " << map.size() << " players" << std::endl;
    player_map_ = map;
}

} // namespace containers

namespace smartptr {

// ================================
// SmartPointerDemo 类实现
// ================================

SmartPointerDemo::SmartPointerDemo() {
    std::cout << "SmartPointerDemo initialized" << std::endl;
}

SmartPointerDemo::~SmartPointerDemo() {
    std::cout << "SmartPointerDemo destroyed" << std::endl;
}

std::shared_ptr<core::Player> SmartPointerDemo::createPlayer(const std::string& name) {
    auto player = std::make_shared<core::Player>(0, name, 1);
    players_.push_back(player);
    return player;
}

std::shared_ptr<core::Entity> SmartPointerDemo::createEntity(int id, const std::string& name) {
    auto entity = std::make_shared<core::Entity>(id, name);
    entities_[id] = entity;
    return entity;
}

void SmartPointerDemo::setCurrentPlayer(std::shared_ptr<core::Player> player) {
    current_player_ = player;
}

std::shared_ptr<core::Player> SmartPointerDemo::getCurrentPlayer() const {
    return current_player_;
}

std::unique_ptr<core::TransformComponent> SmartPointerDemo::createTransform() {
    return std::make_unique<core::TransformComponent>();
}

std::unique_ptr<core::TransformComponent> SmartPointerDemo::createTransform(double x, double y) {
    return std::make_unique<core::TransformComponent>(x, y, 0.0);
}

std::weak_ptr<core::Player> SmartPointerDemo::getPlayerRef(int id) const {
    if (id < static_cast<int>(players_.size())) {
        return players_[id];
    }
    return std::weak_ptr<core::Player>();
}

bool SmartPointerDemo::isPlayerValid(std::weak_ptr<core::Player> player) const {
    return !player.expired();
}

std::vector<std::shared_ptr<core::Player>> SmartPointerDemo::getAllPlayers() const {
    return players_;
}

std::map<int, std::shared_ptr<core::Entity>> SmartPointerDemo::getEntityMap() const {
    return entities_;
}

} // namespace smartptr

} // namespace game

namespace operators {

// ================================
// Vector2D 类实现（运算符重载）
// ================================

Vector2D::Vector2D() : x_(0.0), y_(0.0) {
}

Vector2D::Vector2D(double x, double y) : x_(x), y_(y) {
}

Vector2D::Vector2D(const Vector2D& other) : x_(other.x_), y_(other.y_) {
}

double Vector2D::getX() const {
    return x_;
}

void Vector2D::setX(double x) {
    x_ = x;
}

double Vector2D::getY() const {
    return y_;
}

void Vector2D::setY(double y) {
    y_ = y;
}

Vector2D Vector2D::operator+(const Vector2D& other) const {
    return Vector2D(x_ + other.x_, y_ + other.y_);
}

Vector2D Vector2D::operator-(const Vector2D& other) const {
    return Vector2D(x_ - other.x_, y_ - other.y_);
}

Vector2D Vector2D::operator*(double scalar) const {
    return Vector2D(x_ * scalar, y_ * scalar);
}

Vector2D Vector2D::operator/(double scalar) const {
    if (std::abs(scalar) > 1e-9) {
        return Vector2D(x_ / scalar, y_ / scalar);
    }
    return *this;
}

Vector2D& Vector2D::operator+=(const Vector2D& other) {
    x_ += other.x_;
    y_ += other.y_;
    return *this;
}

Vector2D& Vector2D::operator-=(const Vector2D& other) {
    x_ -= other.x_;
    y_ -= other.y_;
    return *this;
}

Vector2D& Vector2D::operator*=(double scalar) {
    x_ *= scalar;
    y_ *= scalar;
    return *this;
}

bool Vector2D::operator==(const Vector2D& other) const {
    return std::abs(x_ - other.x_) < 1e-9 && std::abs(y_ - other.y_) < 1e-9;
}

bool Vector2D::operator!=(const Vector2D& other) const {
    return !(*this == other);
}

Vector2D Vector2D::operator-() const {
    return Vector2D(-x_, -y_);
}

double Vector2D::operator[](int index) const {
    return (index == 0) ? x_ : y_;
}

double Vector2D::operator()() const {
    return length();
}

double Vector2D::length() const {
    return std::sqrt(x_ * x_ + y_ * y_);
}

double Vector2D::lengthSquared() const {
    return x_ * x_ + y_ * y_;
}

Vector2D Vector2D::normalized() const {
    double len = length();
    if (len > 1e-9) {
        return Vector2D(x_ / len, y_ / len);
    }
    return Vector2D(0, 0);
}

double Vector2D::dot(const Vector2D& other) const {
    return x_ * other.x_ + y_ * other.y_;
}

} // namespace operators

namespace templates {

// ================================
// Container 模板类实现
// ================================

template<typename T>
Container<T>::Container() {
}

template<typename T>
Container<T>::Container(const T& value) : default_value_(value) {
}

template<typename T>
void Container<T>::setValue(const T& value) {
    default_value_ = value;
}

template<typename T>
T Container<T>::getValue() const {
    return default_value_;
}

template<typename T>
void Container<T>::push(const T& item) {
    items_.push_back(item);
}

template<typename T>
T Container<T>::pop() {
    if (!items_.empty()) {
        T item = items_.back();
        items_.pop_back();
        return item;
    }
    return default_value_;
}

template<typename T>
size_t Container<T>::size() const {
    return items_.size();
}

template<typename T>
bool Container<T>::empty() const {
    return items_.empty();
}

// 显式实例化模板类
template class Container<int>;
template class Container<std::string>;
template class Container<double>;

} // namespace templates

namespace factories {

// ================================
// ContainerFactory 类实现
// ================================

ContainerFactory::ContainerFactory() {
    std::cout << "ContainerFactory initialized" << std::endl;
}

ContainerFactory::~ContainerFactory() {
    std::cout << "ContainerFactory destroyed" << std::endl;
}

// ================================
// Vector 工厂方法实现
// ================================

std::vector<int> ContainerFactory::createIntVector() {
    return std::vector<int>();
}

std::vector<int> ContainerFactory::createIntVector(int size, int defaultValue) {
    return std::vector<int>(size, defaultValue);
}

std::vector<std::string> ContainerFactory::createStringVector() {
    return std::vector<std::string>();
}

std::vector<std::string> ContainerFactory::createStringVector(const std::vector<std::string>& elements) {
    return elements; // 创建副本
}

std::vector<double> ContainerFactory::createDoubleVector() {
    return std::vector<double>();
}

std::vector<double> ContainerFactory::createRangeVector(double from, double to, double step) {
    std::vector<double> result;
    if (step <= 0.0) {
        step = 1.0; // 防止无限循环
    }
    
    if (from <= to) {
        for (double val = from; val <= to; val += step) {
            result.push_back(val);
        }
    } else {
        for (double val = from; val >= to; val -= step) {
            result.push_back(val);
        }
    }
    
    return result;
}

// ================================
// Map 工厂方法实现
// ================================

std::map<std::string, int> ContainerFactory::createStringIntMap() {
    return std::map<std::string, int>();
}

std::map<std::string, double> ContainerFactory::createStringDoubleMap() {
    return std::map<std::string, double>();
}

std::map<int, std::string> ContainerFactory::createIntStringMap() {
    return std::map<int, std::string>();
}

std::map<std::string, int> ContainerFactory::createPrefilledStringIntMap() {
    std::map<std::string, int> result;
    result["one"] = 1;
    result["two"] = 2;
    result["three"] = 3;
    result["ten"] = 10;
    result["hundred"] = 100;
    return result;
}

// ================================
// 智能指针容器工厂方法实现
// ================================

std::vector<std::shared_ptr<game::core::Player>> ContainerFactory::createPlayerVector() {
    return std::vector<std::shared_ptr<game::core::Player>>();
}

std::vector<std::shared_ptr<game::core::Player>> ContainerFactory::createPlayerVector(int count) {
    std::vector<std::shared_ptr<game::core::Player>> result;
    result.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        auto player = std::make_shared<game::core::Player>(
            i + 1, 
            "Player_" + std::to_string(i + 1), 
            1 + (i % 10)  // 等级 1-10
        );
        result.push_back(player);
    }
    
    return result;
}

std::map<int, std::shared_ptr<game::core::Player>> ContainerFactory::createPlayerMap() {
    return std::map<int, std::shared_ptr<game::core::Player>>();
}

// ================================
// 嵌套容器工厂方法实现
// ================================

std::vector<std::vector<int>> ContainerFactory::create2DIntVector(int rows, int cols, int defaultValue) {
    std::vector<std::vector<int>> result;
    result.reserve(rows);
    
    for (int i = 0; i < rows; ++i) {
        result.emplace_back(cols, defaultValue);
    }
    
    return result;
}

std::map<std::string, std::vector<int>> ContainerFactory::createMapOfVectors() {
    std::map<std::string, std::vector<int>> result;
    
    result["fibonacci"] = {1, 1, 2, 3, 5, 8, 13, 21};
    result["primes"] = {2, 3, 5, 7, 11, 13, 17, 19};
    result["evens"] = {2, 4, 6, 8, 10, 12, 14, 16};
    result["odds"] = {1, 3, 5, 7, 9, 11, 13, 15};
    result["squares"] = {1, 4, 9, 16, 25, 36, 49, 64};
    
    return result;
}

// ================================
// 容器转换和操作方法实现
// ================================

std::vector<double> ContainerFactory::intVectorToDouble(const std::vector<int>& intVec) {
    std::vector<double> result;
    result.reserve(intVec.size());
    
    for (int val : intVec) {
        result.push_back(static_cast<double>(val));
    }
    
    return result;
}

std::vector<std::string> ContainerFactory::mergeStringVectors(
    const std::vector<std::string>& vec1, 
    const std::vector<std::string>& vec2) {
    
    std::vector<std::string> result;
    result.reserve(vec1.size() + vec2.size());
    
    // 添加第一个 vector 的所有元素
    result.insert(result.end(), vec1.begin(), vec1.end());
    
    // 添加第二个 vector 的所有元素
    result.insert(result.end(), vec2.begin(), vec2.end());
    
    return result;
}

std::vector<std::string> ContainerFactory::extractKeysFromMap(const std::map<std::string, int>& map) {
    std::vector<std::string> result;
    result.reserve(map.size());
    
    for (const auto& pair : map) {
        result.push_back(pair.first);
    }
    
    return result;
}

std::vector<int> ContainerFactory::extractValuesFromMap(const std::map<std::string, int>& map) {
    std::vector<int> result;
    result.reserve(map.size());
    
    for (const auto& pair : map) {
        result.push_back(pair.second);
    }
    
    return result;
}

// ================================
// 容器统计和分析方法实现
// ================================

std::map<std::string, double> ContainerFactory::getVectorStats(const std::vector<int>& vec) {
    std::map<std::string, double> stats;
    
    if (vec.empty()) {
        stats["size"] = 0.0;
        stats["sum"] = 0.0;
        stats["avg"] = 0.0;
        stats["min"] = 0.0;
        stats["max"] = 0.0;
        return stats;
    }
    
    // 计算基本统计信息
    int sum = 0;
    int minVal = vec[0];
    int maxVal = vec[0];
    
    for (int val : vec) {
        sum += val;
        minVal = std::min(minVal, val);
        maxVal = std::max(maxVal, val);
    }
    
    stats["size"] = static_cast<double>(vec.size());
    stats["sum"] = static_cast<double>(sum);
    stats["avg"] = static_cast<double>(sum) / static_cast<double>(vec.size());
    stats["min"] = static_cast<double>(minVal);
    stats["max"] = static_cast<double>(maxVal);
    
    return stats;
}

std::vector<int> ContainerFactory::filterGreaterThan(const std::vector<int>& vec, int threshold) {
    std::vector<int> result;
    
    for (int val : vec) {
        if (val > threshold) {
            result.push_back(val);
        }
    }
    
    return result;
}

std::vector<int> ContainerFactory::sortVector(const std::vector<int>& vec, bool ascending) {
    std::vector<int> result = vec; // 创建副本
    
    if (ascending) {
        std::sort(result.begin(), result.end());
    } else {
        std::sort(result.begin(), result.end(), std::greater<int>());
    }
    
    return result;
}

} // namespace factories