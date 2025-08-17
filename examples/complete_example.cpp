/**
 * @file complete_example.cpp
 * @brief 完整的 Lua 绑定生成器示例实现
 */

#include "complete_example.h"
#include <algorithm>
#include <sstream>
#include <cmath>
#include <random>
#include <cctype>

namespace demo {

// ================================
// 全局函数实现
// ================================

int add(int a, int b) {
    return a + b;
}

std::string greet(const std::string& name) {
    return "Hello, " + name + "!";
}

bool isEven(int number) {
    return (number % 2) == 0;
}

double calculateDistance(double x, double y) {
    return std::sqrt(x * x + y * y);
}

// ================================
// Entity 类实现
// ================================

Entity::Entity() : id_(0), name_("Entity"), status_(Status::INACTIVE) {}

Entity::Entity(int id, const std::string& name) 
    : id_(id), name_(name), status_(Status::ACTIVE) {}

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

Status Entity::getStatus() const {
    return status_;
}

void Entity::setStatus(Status status) {
    status_ = status;
}

std::string Entity::toString() const {
    return "Entity[id=" + std::to_string(id_) + ", name=" + name_ + "]";
}

void Entity::update() {
    // 基础更新逻辑
}

// ================================
// Player 类实现
// ================================

Player::Player() : Entity(), level_(1), health_(100.0), max_health_(100.0), experience_(0) {}

Player::Player(int id, const std::string& name, int level) 
    : Entity(id, name), level_(level), health_(100.0), max_health_(100.0), experience_(0) {}

std::string Player::getType() const {
    return "Player";
}

int Player::getLevel() const {
    return level_;
}

void Player::setLevel(int level) {
    level_ = std::max(1, level);
    max_health_ = 100.0 + (level_ - 1) * 10.0;
    health_ = std::min(health_, max_health_);
}

void Player::levelUp() {
    level_++;
    max_health_ = 100.0 + (level_ - 1) * 10.0;
    health_ = max_health_; // 升级时恢复满血
}

double Player::getHealth() const {
    return health_;
}

void Player::setHealth(double health) {
    health_ = std::clamp(health, 0.0, max_health_);
}

double Player::getMaxHealth() const {
    return max_health_;
}

void Player::heal(double amount) {
    health_ = std::min(health_ + amount, max_health_);
}

void Player::takeDamage(double amount) {
    health_ = std::max(0.0, health_ - amount);
}

int Player::getExperience() const {
    return experience_;
}

void Player::addExperience(int exp) {
    experience_ += exp;
    // 每100经验升一级
    while (experience_ >= level_ * 100) {
        experience_ -= level_ * 100;
        levelUp();
    }
}

void Player::addItem(const std::string& item) {
    items_.push_back(item);
}

std::vector<std::string> Player::getItems() const {
    return items_;
}

bool Player::hasItem(const std::string& item) const {
    return std::find(items_.begin(), items_.end(), item) != items_.end();
}

void Player::update() {
    Entity::update();
    // 玩家特定的更新逻辑
}

std::string Player::toString() const {
    return "Player[id=" + std::to_string(id_) + ", name=" + name_ + 
           ", level=" + std::to_string(level_) + ", health=" + std::to_string(health_) + "]";
}

bool Player::operator==(const Player& other) const {
    return id_ == other.id_;
}

bool Player::operator<(const Player& other) const {
    return id_ < other.id_;
}

// ================================
// MathUtils 类实现
// ================================

double MathUtils::clamp(double value, double min_val, double max_val) {
    return std::max(min_val, std::min(value, max_val));
}

double MathUtils::lerp(double a, double b, double t) {
    return a + t * (b - a);
}

int MathUtils::randomInt(int min_val, int max_val) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min_val, max_val);
    return dis(gen);
}

double MathUtils::randomDouble(double min_val, double max_val) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min_val, max_val);
    return dis(gen);
}

double MathUtils::distance2D(double x1, double y1, double x2, double y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

bool MathUtils::isPrime(int number) {
    if (number < 2) return false;
    if (number == 2) return true;
    if (number % 2 == 0) return false;
    
    for (int i = 3; i * i <= number; i += 2) {
        if (number % i == 0) return false;
    }
    return true;
}

int MathUtils::factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

double MathUtils::toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

double MathUtils::toDegrees(double radians) {
    return radians * 180.0 / M_PI;
}

// ================================
// StringUtils 类实现
// ================================

std::string StringUtils::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string StringUtils::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string StringUtils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> StringUtils::split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }
    
    return result;
}

std::string StringUtils::join(const std::vector<std::string>& parts, const std::string& separator) {
    if (parts.empty()) return "";
    
    std::string result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result += separator + parts[i];
    }
    
    return result;
}

bool StringUtils::startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 
           str.substr(0, prefix.size()) == prefix;
}

bool StringUtils::endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 
           str.substr(str.size() - suffix.size()) == suffix;
}

std::string StringUtils::reverse(const std::string& str) {
    std::string result = str;
    std::reverse(result.begin(), result.end());
    return result;
}

int StringUtils::count(const std::string& str, char ch) {
    return std::count(str.begin(), str.end(), ch);
}

// ================================
// GameManager 单例实现
// ================================

GameManager& GameManager::getInstance() {
    static GameManager instance;
    return instance;
}

void GameManager::startGame() {
    game_running_ = true;
    game_paused_ = false;
    game_time_ = 0.0;
}

void GameManager::pauseGame() {
    if (game_running_) {
        game_paused_ = true;
    }
}

void GameManager::stopGame() {
    game_running_ = false;
    game_paused_ = false;
}

bool GameManager::isGameRunning() const {
    return game_running_ && !game_paused_;
}

bool GameManager::isGamePaused() const {
    return game_paused_;
}

void GameManager::addPlayer(std::shared_ptr<Player> player) {
    if (player) {
        players_.push_back(player);
    }
}

void GameManager::removePlayer(int playerId) {
    auto it = std::remove_if(players_.begin(), players_.end(),
        [playerId](const std::shared_ptr<Player>& player) {
            return player && player->getId() == playerId;
        });
    players_.erase(it, players_.end());
}

std::shared_ptr<Player> GameManager::getPlayer(int playerId) {
    auto it = std::find_if(players_.begin(), players_.end(),
        [playerId](const std::shared_ptr<Player>& player) {
            return player && player->getId() == playerId;
        });
    return (it != players_.end()) ? *it : nullptr;
}

std::vector<std::shared_ptr<Player>> GameManager::getAllPlayers() {
    return players_;
}

int GameManager::getPlayerCount() const {
    return static_cast<int>(players_.size());
}

int GameManager::getScore() const {
    return score_;
}

void GameManager::addScore(int points) {
    score_ += points;
}

double GameManager::getTime() const {
    return game_time_;
}

void GameManager::updateTime(double deltaTime) {
    if (game_running_ && !game_paused_) {
        game_time_ += deltaTime;
    }
}

void GameManager::setSetting(const std::string& key, const std::string& value) {
    settings_[key] = value;
}

std::string GameManager::getSetting(const std::string& key) const {
    auto it = settings_.find(key);
    return (it != settings_.end()) ? it->second : "";
}

// ================================
// ContainerManager 类实现
// ================================

ContainerManager::ContainerManager() {}

void ContainerManager::addNumber(int number) {
    numbers_.push_back(number);
}

std::vector<int> ContainerManager::getNumbers() const {
    return numbers_;
}

void ContainerManager::clearNumbers() {
    numbers_.clear();
}

int ContainerManager::getNumberAt(int index) const {
    if (index >= 0 && index < static_cast<int>(numbers_.size())) {
        return numbers_[index];
    }
    return 0;
}

int ContainerManager::getNumberCount() const {
    return static_cast<int>(numbers_.size());
}

void ContainerManager::setProperty(const std::string& key, const std::string& value) {
    properties_[key] = value;
}

std::string ContainerManager::getProperty(const std::string& key) const {
    auto it = properties_.find(key);
    return (it != properties_.end()) ? it->second : "";
}

std::map<std::string, std::string> ContainerManager::getAllProperties() const {
    return properties_;
}

bool ContainerManager::hasProperty(const std::string& key) const {
    return properties_.find(key) != properties_.end();
}

void ContainerManager::removeProperty(const std::string& key) {
    properties_.erase(key);
}

void ContainerManager::addPlayerScore(const std::string& playerName, int score) {
    player_scores_[playerName] = score;
}

std::map<std::string, int> ContainerManager::getPlayerScores() const {
    return player_scores_;
}

std::vector<std::string> ContainerManager::getTopPlayers(int count) const {
    std::vector<std::pair<std::string, int>> scores(player_scores_.begin(), player_scores_.end());
    
    std::sort(scores.begin(), scores.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::vector<std::string> result;
    for (int i = 0; i < std::min(count, static_cast<int>(scores.size())); ++i) {
        result.push_back(scores[i].first);
    }
    
    return result;
}

// ================================
// Vector2D 类实现
// ================================

Vector2D::Vector2D() : x_(0.0), y_(0.0) {}

Vector2D::Vector2D(double x, double y) : x_(x), y_(y) {}

double Vector2D::getX() const {
    return x_;
}

double Vector2D::getY() const {
    return y_;
}

void Vector2D::setX(double x) {
    x_ = x;
}

void Vector2D::setY(double y) {
    y_ = y;
}

double Vector2D::length() const {
    return std::sqrt(x_ * x_ + y_ * y_);
}

double Vector2D::lengthSquared() const {
    return x_ * x_ + y_ * y_;
}

Vector2D Vector2D::normalized() const {
    double len = length();
    if (len > 0.0) {
        return Vector2D(x_ / len, y_ / len);
    }
    return Vector2D();
}

double Vector2D::dot(const Vector2D& other) const {
    return x_ * other.x_ + y_ * other.y_;
}

double Vector2D::cross(const Vector2D& other) const {
    return x_ * other.y_ - y_ * other.x_;
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
    if (scalar != 0.0) {
        return Vector2D(x_ / scalar, y_ / scalar);
    }
    return *this;
}

bool Vector2D::operator==(const Vector2D& other) const {
    const double epsilon = 1e-9;
    return std::abs(x_ - other.x_) < epsilon && std::abs(y_ - other.y_) < epsilon;
}

bool Vector2D::operator!=(const Vector2D& other) const {
    return !(*this == other);
}

double Vector2D::operator[](int index) const {
    return (index == 0) ? x_ : (index == 1) ? y_ : 0.0;
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

Vector2D& Vector2D::operator/=(double scalar) {
    if (scalar != 0.0) {
        x_ /= scalar;
        y_ /= scalar;
    }
    return *this;
}

std::string Vector2D::toString() const {
    return "Vector2D(" + std::to_string(x_) + ", " + std::to_string(y_) + ")";
}

// ================================
// EventSystem 类实现
// ================================

EventSystem::EventSystem() {}

void EventSystem::triggerGameStart() {
    if (OnGameStart) {
        OnGameStart();
    }
}

void EventSystem::triggerScoreChange(int newScore) {
    if (OnScoreChange) {
        OnScoreChange(newScore);
    }
    last_score_ = newScore;
}

void EventSystem::triggerPlayerJoin(std::shared_ptr<Player> player) {
    if (OnPlayerJoin && player) {
        OnPlayerJoin(player);
    }
}

bool EventSystem::validateAction(const std::string& action, int value) {
    if (OnValidateAction) {
        return OnValidateAction(action, value);
    }
    return true; // 默认允许所有操作
}

bool EventSystem::hasGameStartCallback() const {
    return static_cast<bool>(OnGameStart);
}

bool EventSystem::hasScoreChangeCallback() const {
    return static_cast<bool>(OnScoreChange);
}

bool EventSystem::hasPlayerJoinCallback() const {
    return static_cast<bool>(OnPlayerJoin);
}

bool EventSystem::hasValidateActionCallback() const {
    return static_cast<bool>(OnValidateAction);
}

// ================================
// ResourceManager 类实现
// ================================

ResourceManager::ResourceManager() {}

ResourceManager::~ResourceManager() {
    cleanup();
}

std::shared_ptr<Player> ResourceManager::createPlayer(const std::string& name) {
    auto player = std::make_shared<Player>(next_id_++, name, 1);
    players_.push_back(player);
    return player;
}

void ResourceManager::addPlayer(std::shared_ptr<Player> player) {
    if (player) {
        players_.push_back(player);
    }
}

std::shared_ptr<Player> ResourceManager::getPlayer(int id) {
    auto it = std::find_if(players_.begin(), players_.end(),
        [id](const std::shared_ptr<Player>& player) {
            return player && player->getId() == id;
        });
    return (it != players_.end()) ? *it : nullptr;
}

std::vector<std::shared_ptr<Player>> ResourceManager::getAllPlayers() {
    return players_;
}

void ResourceManager::removePlayer(int id) {
    auto it = std::remove_if(players_.begin(), players_.end(),
        [id](const std::shared_ptr<Player>& player) {
            return player && player->getId() == id;
        });
    players_.erase(it, players_.end());
}

int ResourceManager::getPlayerCount() const {
    return static_cast<int>(players_.size());
}

std::vector<std::string> ResourceManager::getPlayerNames() const {
    std::vector<std::string> names;
    for (const auto& player : players_) {
        if (player) {
            names.push_back(player->getName());
        }
    }
    return names;
}

void ResourceManager::cleanup() {
    players_.clear();
}

std::string ResourceManager::getInfo() const {
    return "ResourceManager managing " + std::to_string(players_.size()) + " players";
}

} // namespace demo