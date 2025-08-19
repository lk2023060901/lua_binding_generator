/**
 * @file macro_coverage.cpp
 * @brief 15个核心宏覆盖测试的实现
 */

#include "../headers/macro_coverage.h"
#include <algorithm>
#include <numeric>
#include <random>
#include <sstream>
#include <iomanip>

namespace test_coverage {

// ================================
// 全局函数实现
// ================================

int add_numbers(int a, int b) {
    return a + b;
}

std::string format_message(const std::string& template_str, const std::string& value) {
    std::string result = template_str;
    size_t pos = result.find("{}");
    if (pos != std::string::npos) {
        result.replace(pos, 2, value);
    }
    return result;
}

std::vector<int> generate_sequence(int start, int end, int step) {
    std::vector<int> result;
    if (step > 0) {
        for (int i = start; i <= end; i += step) {
            result.push_back(i);
        }
    } else if (step < 0) {
        for (int i = start; i >= end; i += step) {
            result.push_back(i);
        }
    }
    return result;
}

double calculate_area(double radius) {
    return PI_VALUE * radius * radius;
}

double calculate_area(double width, double height) {
    return width * height;
}

void process_items(const std::vector<int>& items, std::function<void(int)> processor) {
    for (int item : items) {
        if (processor) {
            processor(item);
        }
    }
}

// ================================
// TestEntity (抽象类) 实现
// ================================

TestEntity::TestEntity() : id_(0), name_("DefaultEntity"), status_(TestStatus::INACTIVE) {}

TestEntity::TestEntity(int id, const std::string& name) 
    : id_(id), name_(name), status_(TestStatus::ACTIVE) {}

int TestEntity::getId() const { return id_; }
void TestEntity::setId(int id) { id_ = id; }

std::string TestEntity::getName() const { return name_; }
void TestEntity::setName(const std::string& name) { name_ = name; }

TestStatus TestEntity::getStatus() const { return status_; }
void TestEntity::setStatus(TestStatus status) { status_ = status; }

std::string TestEntity::toString() const {
    return "Entity{id=" + std::to_string(id_) + ", name=" + name_ + ", status=" + std::to_string(static_cast<int>(status_)) + "}";
}

void TestEntity::update() {
    // 基础更新逻辑
}

// ================================
// TestPlayer 实现
// ================================

TestPlayer::TestPlayer() 
    : TestEntity(0, "DefaultPlayer"), level_(1), health_(100.0), max_health_(100.0), experience_(0) {}

TestPlayer::TestPlayer(const std::string& name, int level)
    : TestEntity(0, name), level_(level), health_(100.0), max_health_(100.0), experience_(0) {}

TestPlayer::TestPlayer(int id, const std::string& name, int level)
    : TestEntity(id, name), level_(level), health_(100.0), max_health_(100.0), experience_(0) {}

int TestPlayer::getLevel() const { return level_; }
void TestPlayer::setLevel(int level) { 
    level_ = std::max(1, level);
    max_health_ = 100.0 + (level_ - 1) * 10.0;
    if (health_ > max_health_) {
        health_ = max_health_;
    }
}

void TestPlayer::levelUp() {
    level_++;
    max_health_ = 100.0 + (level_ - 1) * 10.0;
    health_ = max_health_; // 满血升级
    experience_ = 0; // 重置经验
}

double TestPlayer::getHealth() const { return health_; }
void TestPlayer::setHealth(double health) { 
    health_ = std::clamp(health, 0.0, max_health_);
}

double TestPlayer::getMaxHealth() const { return max_health_; }

void TestPlayer::heal(double amount) {
    health_ = std::min(health_ + amount, max_health_);
}

void TestPlayer::takeDamage(double amount) {
    health_ = std::max(health_ - amount, 0.0);
    if (health_ <= 0) {
        setStatus(TestStatus::ERROR); // 死亡状态
    }
}

int TestPlayer::getExperience() const { return experience_; }

void TestPlayer::addExperience(int exp) {
    experience_ += exp;
    while (experience_ >= 100) { // 每100经验升级
        experience_ -= 100;
        levelUp();
    }
}

void TestPlayer::addItem(const std::string& item) {
    items_.push_back(item);
}

std::vector<std::string> TestPlayer::getItems() const {
    return items_;
}

bool TestPlayer::hasItem(const std::string& item) const {
    return std::find(items_.begin(), items_.end(), item) != items_.end();
}

void TestPlayer::removeItem(const std::string& item) {
    auto it = std::find(items_.begin(), items_.end(), item);
    if (it != items_.end()) {
        items_.erase(it);
    }
}

void TestPlayer::update() {
    TestEntity::update();
    // 玩家特定的更新逻辑
    if (getStatus() == TestStatus::ACTIVE && health_ > 0) {
        // 缓慢恢复生命值
        heal(0.1);
    }
}

std::string TestPlayer::getType() const {
    return "Player";
}

double TestPlayer::getScore() const {
    return level_ * 100 + experience_ + items_.size() * 10;
}

bool TestPlayer::operator==(const TestPlayer& other) const {
    return getId() == other.getId();
}

bool TestPlayer::operator<(const TestPlayer& other) const {
    return level_ < other.level_ || (level_ == other.level_ && getId() < other.getId());
}

std::ostream& operator<<(std::ostream& os, const TestPlayer& player) {
    os << "Player{" << player.getName() << ", Lv." << player.getLevel() << ", HP:" << player.getHealth() << "}";
    return os;
}

// ================================
// TestManager 实现
// ================================

TestManager::TestManager() : next_id_(1) {}

TestManager::~TestManager() {
    clearAll();
}

void TestManager::addPlayer(std::shared_ptr<TestPlayer> player) {
    if (player) {
        player->setId(next_id_++);
        players_.push_back(player);
    }
}

void TestManager::removePlayer(int playerId) {
    players_.erase(
        std::remove_if(players_.begin(), players_.end(),
            [playerId](const std::shared_ptr<TestPlayer>& p) {
                return p && p->getId() == playerId;
            }),
        players_.end()
    );
}

std::shared_ptr<TestPlayer> TestManager::getPlayer(int playerId) {
    auto it = std::find_if(players_.begin(), players_.end(),
        [playerId](const std::shared_ptr<TestPlayer>& p) {
            return p && p->getId() == playerId;
        });
    return (it != players_.end()) ? *it : nullptr;
}

std::vector<std::shared_ptr<TestPlayer>> TestManager::getAllPlayers() {
    return players_;
}

int TestManager::getPlayerCount() const {
    return static_cast<int>(players_.size());
}

double TestManager::getAverageLevel() const {
    if (players_.empty()) return 0.0;
    
    double total = 0.0;
    for (const auto& player : players_) {
        if (player) {
            total += player->getLevel();
        }
    }
    return total / players_.size();
}

TestPlayer* TestManager::getTopPlayer() const {
    if (players_.empty()) return nullptr;
    
    auto it = std::max_element(players_.begin(), players_.end(),
        [](const std::shared_ptr<TestPlayer>& a, const std::shared_ptr<TestPlayer>& b) {
            return a && b && (a->getLevel() < b->getLevel());
        });
    
    return (it != players_.end() && *it) ? it->get() : nullptr;
}

std::vector<std::string> TestManager::getPlayerNames() const {
    std::vector<std::string> names;
    for (const auto& player : players_) {
        if (player) {
            names.push_back(player->getName());
        }
    }
    return names;
}

void TestManager::levelUpAll() {
    for (auto& player : players_) {
        if (player) {
            player->levelUp();
        }
    }
}

void TestManager::healAll(double amount) {
    for (auto& player : players_) {
        if (player) {
            player->heal(amount);
        }
    }
}

void TestManager::clearAll() {
    players_.clear();
    next_id_ = 1;
}

// ================================
// TestMathUtils 静态类实现
// ================================

double TestMathUtils::clamp(double value, double min_val, double max_val) {
    return std::max(min_val, std::min(value, max_val));
}

double TestMathUtils::lerp(double a, double b, double t) {
    return a + t * (b - a);
}

int TestMathUtils::randomInt(int min_val, int max_val) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(min_val, max_val);
    return dis(gen);
}

double TestMathUtils::randomDouble(double min_val, double max_val) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(min_val, max_val);
    return dis(gen);
}

double TestMathUtils::distance2D(double x1, double y1, double x2, double y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

double TestMathUtils::distance3D(double x1, double y1, double z1, double x2, double y2, double z2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    double dz = z2 - z1;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool TestMathUtils::isPrime(int number) {
    if (number < 2) return false;
    if (number == 2) return true;
    if (number % 2 == 0) return false;
    
    for (int i = 3; i * i <= number; i += 2) {
        if (number % i == 0) return false;
    }
    return true;
}

int TestMathUtils::factorial(int n) {
    if (n < 0) return 0;
    if (n <= 1) return 1;
    
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

double TestMathUtils::toRadians(double degrees) {
    return degrees * PI_VALUE / 180.0;
}

double TestMathUtils::toDegrees(double radians) {
    return radians * 180.0 / PI_VALUE;
}

std::vector<int> TestMathUtils::generatePrimes(int max_num) {
    std::vector<int> primes;
    for (int i = 2; i <= max_num; ++i) {
        if (isPrime(i)) {
            primes.push_back(i);
        }
    }
    return primes;
}

// ================================
// TestStringUtils 静态类实现
// ================================

std::string TestStringUtils::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string TestStringUtils::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string TestStringUtils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> TestStringUtils::split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }
    return result;
}

std::string TestStringUtils::join(const std::vector<std::string>& parts, const std::string& separator) {
    if (parts.empty()) return "";
    
    std::string result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result += separator + parts[i];
    }
    return result;
}

bool TestStringUtils::startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix;
}

bool TestStringUtils::endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix;
}

std::string TestStringUtils::reverse(const std::string& str) {
    std::string result = str;
    std::reverse(result.begin(), result.end());
    return result;
}

int TestStringUtils::count(const std::string& str, char ch) {
    return static_cast<int>(std::count(str.begin(), str.end(), ch));
}

std::string TestStringUtils::replace(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

// ================================
// TestGameManager 单例实现
// ================================

TestGameManager& TestGameManager::getInstance() {
    static TestGameManager instance;
    return instance;
}

void TestGameManager::startGame() {
    game_running_ = true;
    game_paused_ = false;
    game_time_ = 0.0;
}

void TestGameManager::pauseGame() {
    if (game_running_) {
        game_paused_ = true;
    }
}

void TestGameManager::stopGame() {
    game_running_ = false;
    game_paused_ = false;
}

bool TestGameManager::isGameRunning() const {
    return game_running_;
}

bool TestGameManager::isGamePaused() const {
    return game_paused_;
}

void TestGameManager::addPlayer(std::shared_ptr<TestPlayer> player) {
    if (player) {
        players_.push_back(player);
    }
}

void TestGameManager::removePlayer(int playerId) {
    players_.erase(
        std::remove_if(players_.begin(), players_.end(),
            [playerId](const std::shared_ptr<TestPlayer>& p) {
                return p && p->getId() == playerId;
            }),
        players_.end()
    );
}

std::shared_ptr<TestPlayer> TestGameManager::getPlayer(int playerId) {
    auto it = std::find_if(players_.begin(), players_.end(),
        [playerId](const std::shared_ptr<TestPlayer>& p) {
            return p && p->getId() == playerId;
        });
    return (it != players_.end()) ? *it : nullptr;
}

std::vector<std::shared_ptr<TestPlayer>> TestGameManager::getAllPlayers() {
    return players_;
}

int TestGameManager::getPlayerCount() const {
    return static_cast<int>(players_.size());
}

int TestGameManager::getScore() const {
    return score_;
}

void TestGameManager::addScore(int points) {
    score_ += points;
}

double TestGameManager::getTime() const {
    return game_time_;
}

void TestGameManager::updateTime(double deltaTime) {
    if (game_running_ && !game_paused_) {
        game_time_ += deltaTime;
    }
}

void TestGameManager::resetGame() {
    score_ = 0;
    game_time_ = 0.0;
    players_.clear();
    settings_.clear();
}

void TestGameManager::setSetting(const std::string& key, const std::string& value) {
    settings_[key] = value;
}

std::string TestGameManager::getSetting(const std::string& key) const {
    auto it = settings_.find(key);
    return (it != settings_.end()) ? it->second : "";
}

bool TestGameManager::hasSetting(const std::string& key) const {
    return settings_.find(key) != settings_.end();
}

void TestGameManager::clearSettings() {
    settings_.clear();
}

// ================================
// TestVector2D 运算符重载实现
// ================================

TestVector2D::TestVector2D() : x_(0.0), y_(0.0) {}

TestVector2D::TestVector2D(double x, double y) : x_(x), y_(y) {}

TestVector2D::TestVector2D(const TestVector2D& other) : x_(other.x_), y_(other.y_) {}

double TestVector2D::getX() const { return x_; }
double TestVector2D::getY() const { return y_; }
void TestVector2D::setX(double x) { x_ = x; }
void TestVector2D::setY(double y) { y_ = y; }

double TestVector2D::length() const {
    return std::sqrt(x_ * x_ + y_ * y_);
}

double TestVector2D::lengthSquared() const {
    return x_ * x_ + y_ * y_;
}

TestVector2D TestVector2D::normalized() const {
    double len = length();
    if (len > 0.0) {
        return TestVector2D(x_ / len, y_ / len);
    }
    return TestVector2D(0.0, 0.0);
}

double TestVector2D::dot(const TestVector2D& other) const {
    return x_ * other.x_ + y_ * other.y_;
}

double TestVector2D::cross(const TestVector2D& other) const {
    return x_ * other.y_ - y_ * other.x_;
}

double TestVector2D::distance(const TestVector2D& other) const {
    double dx = x_ - other.x_;
    double dy = y_ - other.y_;
    return std::sqrt(dx * dx + dy * dy);
}

TestVector2D TestVector2D::operator+(const TestVector2D& other) const {
    return TestVector2D(x_ + other.x_, y_ + other.y_);
}

TestVector2D TestVector2D::operator-(const TestVector2D& other) const {
    return TestVector2D(x_ - other.x_, y_ - other.y_);
}

TestVector2D TestVector2D::operator*(double scalar) const {
    return TestVector2D(x_ * scalar, y_ * scalar);
}

TestVector2D TestVector2D::operator/(double scalar) const {
    if (scalar != 0.0) {
        return TestVector2D(x_ / scalar, y_ / scalar);
    }
    return TestVector2D(0.0, 0.0);
}

bool TestVector2D::operator==(const TestVector2D& other) const {
    const double epsilon = 1e-9;
    return std::abs(x_ - other.x_) < epsilon && std::abs(y_ - other.y_) < epsilon;
}

bool TestVector2D::operator!=(const TestVector2D& other) const {
    return !(*this == other);
}

bool TestVector2D::operator<(const TestVector2D& other) const {
    return lengthSquared() < other.lengthSquared();
}

double TestVector2D::operator[](int index) const {
    return (index == 0) ? x_ : (index == 1) ? y_ : 0.0;
}

TestVector2D& TestVector2D::operator+=(const TestVector2D& other) {
    x_ += other.x_;
    y_ += other.y_;
    return *this;
}

TestVector2D& TestVector2D::operator-=(const TestVector2D& other) {
    x_ -= other.x_;
    y_ -= other.y_;
    return *this;
}

TestVector2D& TestVector2D::operator*=(double scalar) {
    x_ *= scalar;
    y_ *= scalar;
    return *this;
}

TestVector2D& TestVector2D::operator/=(double scalar) {
    if (scalar != 0.0) {
        x_ /= scalar;
        y_ /= scalar;
    }
    return *this;
}

TestVector2D TestVector2D::operator-() const {
    return TestVector2D(-x_, -y_);
}

TestVector2D& TestVector2D::operator=(const TestVector2D& other) {
    if (this != &other) {
        x_ = other.x_;
        y_ = other.y_;
    }
    return *this;
}

std::string TestVector2D::toString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << "(" << x_ << ", " << y_ << ")";
    return oss.str();
}

void TestVector2D::zero() {
    x_ = 0.0;
    y_ = 0.0;
}

void TestVector2D::normalize() {
    double len = length();
    if (len > 0.0) {
        x_ /= len;
        y_ /= len;
    }
}

// ================================
// TestEventSystem 回调实现
// ================================

TestEventSystem::TestEventSystem() {
    // 初始化回调为空
}

TestEventSystem::~TestEventSystem() {
    clearAllCallbacks();
}

void TestEventSystem::triggerGameStart() {
    if (OnGameStart) {
        OnGameStart();
    }
}

void TestEventSystem::triggerScoreChange(int newScore) {
    last_score_ = newScore;
    if (OnScoreChange) {
        OnScoreChange(newScore);
    }
}

void TestEventSystem::triggerPlayerJoin(std::shared_ptr<TestPlayer> player) {
    if (OnPlayerJoin && player) {
        OnPlayerJoin(player);
    }
}

bool TestEventSystem::validateAction(const std::string& action, int value) {
    if (OnValidateAction) {
        return OnValidateAction(action, value);
    }
    return true; // 默认允许
}

void TestEventSystem::triggerPositionChange(double x, double y) {
    last_x_ = x;
    last_y_ = y;
    if (OnPositionChange) {
        OnPositionChange(x, y);
    }
}

std::string TestEventSystem::filterMessage(const std::string& message) {
    if (OnMessageFilter) {
        return OnMessageFilter(message);
    }
    return message; // 默认不过滤
}

bool TestEventSystem::hasGameStartCallback() const {
    return static_cast<bool>(OnGameStart);
}

bool TestEventSystem::hasScoreChangeCallback() const {
    return static_cast<bool>(OnScoreChange);
}

bool TestEventSystem::hasPlayerJoinCallback() const {
    return static_cast<bool>(OnPlayerJoin);
}

bool TestEventSystem::hasValidateActionCallback() const {
    return static_cast<bool>(OnValidateAction);
}

void TestEventSystem::clearAllCallbacks() {
    OnGameStart = nullptr;
    OnScoreChange = nullptr;
    OnPlayerJoin = nullptr;
    OnValidateAction = nullptr;
    OnPositionChange = nullptr;
    OnMessageFilter = nullptr;
}

void TestEventSystem::triggerMultipleEvents() {
    triggerGameStart();
    triggerScoreChange(100);
    triggerPositionChange(10.0, 20.0);
}

// ================================
// TestContainerManager 容器实现
// ================================

TestContainerManager::TestContainerManager() {}

TestContainerManager::~TestContainerManager() {}

void TestContainerManager::addNumber(int number) {
    numbers_.push_back(number);
}

void TestContainerManager::addNumbers(const std::vector<int>& numbers) {
    numbers_.insert(numbers_.end(), numbers.begin(), numbers.end());
}

std::vector<int> TestContainerManager::getNumbers() const {
    return numbers_;
}

void TestContainerManager::clearNumbers() {
    numbers_.clear();
}

int TestContainerManager::getNumberAt(int index) const {
    if (index >= 0 && index < static_cast<int>(numbers_.size())) {
        return numbers_[index];
    }
    return 0;
}

int TestContainerManager::getNumberCount() const {
    return static_cast<int>(numbers_.size());
}

void TestContainerManager::removeNumberAt(int index) {
    if (index >= 0 && index < static_cast<int>(numbers_.size())) {
        numbers_.erase(numbers_.begin() + index);
    }
}

void TestContainerManager::addString(const std::string& str) {
    strings_.push_back(str);
}

std::vector<std::string> TestContainerManager::getStrings() const {
    return strings_;
}

std::string TestContainerManager::getStringAt(int index) const {
    if (index >= 0 && index < static_cast<int>(strings_.size())) {
        return strings_[index];
    }
    return "";
}

void TestContainerManager::setProperty(const std::string& key, const std::string& value) {
    properties_[key] = value;
}

std::string TestContainerManager::getProperty(const std::string& key) const {
    auto it = properties_.find(key);
    return (it != properties_.end()) ? it->second : "";
}

std::map<std::string, std::string> TestContainerManager::getAllProperties() const {
    return properties_;
}

bool TestContainerManager::hasProperty(const std::string& key) const {
    return properties_.find(key) != properties_.end();
}

void TestContainerManager::removeProperty(const std::string& key) {
    properties_.erase(key);
}

std::vector<std::string> TestContainerManager::getPropertyKeys() const {
    std::vector<std::string> keys;
    for (const auto& pair : properties_) {
        keys.push_back(pair.first);
    }
    return keys;
}

void TestContainerManager::addPlayerScore(const std::string& playerName, int score) {
    player_scores_[playerName] = score;
}

std::map<std::string, int> TestContainerManager::getPlayerScores() const {
    return player_scores_;
}

std::vector<std::string> TestContainerManager::getTopPlayers(int count) const {
    std::vector<std::pair<std::string, int>> players(player_scores_.begin(), player_scores_.end());
    
    std::sort(players.begin(), players.end(),
        [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
            return a.second > b.second;
        });
    
    std::vector<std::string> top_players;
    int limit = std::min(count, static_cast<int>(players.size()));
    for (int i = 0; i < limit; ++i) {
        top_players.push_back(players[i].first);
    }
    return top_players;
}

int TestContainerManager::getPlayerScore(const std::string& playerName) const {
    auto it = player_scores_.find(playerName);
    return (it != player_scores_.end()) ? it->second : 0;
}

void TestContainerManager::addPlayerToList(std::shared_ptr<TestPlayer> player) {
    if (player) {
        player_list_.push_back(player);
    }
}

std::vector<std::shared_ptr<TestPlayer>> TestContainerManager::getPlayerList() const {
    return player_list_;
}

void TestContainerManager::clearPlayerList() {
    player_list_.clear();
}

} // namespace test_coverage