# Lua Binding Generator

[English](./README_EN.md) | [中文](./README.md)

## Overview

Lua Binding Generator is a modern C++ to Lua binding generation tool designed to simplify development workflows. Through intelligent AST analysis and zero-configuration philosophy, it makes C++ code Lua binding generation unprecedentedly simple and efficient.

### Why Choose Lua Binding Generator?

🚀 **Zero Configuration** - Most use cases need only parameter-free macros, say goodbye to tedious configurations  
🧠 **Smart Inference** - Automatically infer required information from AST, significantly reducing manual input  
⚡ **Ultimate Performance** - Hard-coded generator, efficient and fast  
🔄 **Incremental Compilation** - Smart caching mechanism, significantly reducing regeneration time  
🎯 **Full Support** - Covers all modern C++ features including templates, STL, callbacks, etc.  
🛠️ **Completely Self-Contained** - Built-in all necessary third-party libraries, no additional installation required

### Quick Start

```cpp
// 1. Include header
#include "export_macros.h"

// 2. Define module
EXPORT_LUA_MODULE(GameCore)

// 3. Export class (automatically infer all public members)
class EXPORT_LUA_CLASS() Player {
public:
    Player(const std::string& name, int level);
    
    // These methods will be exported automatically, no additional configuration needed
    std::string getName() const;        // Automatically inferred as read-only property "name"
    void setName(const std::string& name);
    int getLevel() const;               // Automatically inferred as read-only property "level"
    void levelUp();
};

// 4. Export function
EXPORT_LUA_FUNCTION()
double calculateDistance(double x1, double y1, double x2, double y2);

// 5. Generate bindings
// ./lua_binding_generator examples/your_code.h
```

### Core Features

🔍 **Automatic Inference**:
- Class names, method names, function names automatically extracted from code
- C++ namespaces automatically mapped to Lua namespaces  
- get/set methods automatically paired as Lua properties
- STL containers and template parameters automatically recognized

⚡ **Incremental Compilation**:
- Smart caching based on file content hashes
- Only regenerate changed files
- Significantly reduce regeneration time in large projects

🎯 **Parallel Processing**:
- Multi-threaded parallel analysis and generation
- Smart task allocation and load balancing

🔄 **Runtime Library Integration**:
- Complete Lua runtime manager
- Memory allocators and performance monitoring
- Hot reload system support
- Error handling and debugging tools

## Supported Macro List

### Core Macros (15)

| Macro Name | Purpose | Example |
|------------|---------|---------|
| `EXPORT_LUA_MODULE` | Module definition | `EXPORT_LUA_MODULE(GameCore)` |
| `EXPORT_LUA_CLASS` | Regular class export | `class EXPORT_LUA_CLASS() Player {}` |
| `EXPORT_LUA_SINGLETON` | Singleton class export | `class EXPORT_LUA_SINGLETON() GameManager {}` |
| `EXPORT_LUA_STATIC_CLASS` | Static class export | `class EXPORT_LUA_STATIC_CLASS() MathUtils {}` |
| `EXPORT_LUA_ABSTRACT_CLASS` | Abstract class export | `class EXPORT_LUA_ABSTRACT_CLASS() Component {}` |
| `EXPORT_LUA_ENUM` | Enum export | `enum class EXPORT_LUA_ENUM() Status {}` |
| `EXPORT_LUA_FUNCTION` | Function export | `EXPORT_LUA_FUNCTION() int calc();` |
| `EXPORT_LUA_VARIABLE` | Variable export | `EXPORT_LUA_VARIABLE() static int level;` |
| `EXPORT_LUA_CONSTANT` | Constant export | `EXPORT_LUA_CONSTANT() const int MAX = 100;` |
| `EXPORT_LUA_VECTOR` | Vector container | `EXPORT_LUA_VECTOR(int)` |
| `EXPORT_LUA_MAP` | Map container | `EXPORT_LUA_MAP(std::string, int)` |
| `EXPORT_LUA_CALLBACK` | Callback function export | `EXPORT_LUA_CALLBACK() std::function<void()> cb;` |
| `EXPORT_LUA_OPERATOR` | Operator export | `EXPORT_LUA_OPERATOR(+) Vector operator+();` |
| `EXPORT_LUA_TEMPLATE` | Template class export | `class EXPORT_LUA_TEMPLATE(T) Container {}` |
| `EXPORT_LUA_IGNORE` | Ignore export | `EXPORT_LUA_IGNORE() void internal();` |

> **Note**: Abstract classes are usually handled through smart inheritance mechanism, automatically adding base class methods to derived class bindings

### Convenience Macros

| Macro Name | Purpose | Equivalent To |
|------------|---------|---------------|
| `EXPORT_LUA_READONLY_PROPERTY` | Read-only property | `EXPORT_LUA_PROPERTY(access=readonly)` |
| `EXPORT_LUA_READWRITE_PROPERTY` | Read-write property | `EXPORT_LUA_PROPERTY(access=readwrite)` |

## Detailed Usage Examples

### Zero Configuration Usage Example

The following example demonstrates the zero-configuration usage:

```cpp
#include "export_macros.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

// 1. Module definition - file-level setting
EXPORT_LUA_MODULE(GameCore)

namespace game {

// 2. Enum - automatically export all values
enum class EXPORT_LUA_ENUM() PlayerStatus {
    ALIVE,      // Automatically exported as PlayerStatus.ALIVE
    DEAD,       // Automatically exported as PlayerStatus.DEAD
    RESPAWNING  // Automatically exported as PlayerStatus.RESPAWNING
};

// 3. Basic class - automatically export all public members
class EXPORT_LUA_CLASS() Player {
public:
    // Constructors - automatically exported
    Player();
    Player(const std::string& name, int level);
    
    // Property methods - automatically recognized as Lua properties
    std::string getName() const;           // Automatically inferred as read-only property "name"
    void setName(const std::string& name);
    
    int getLevel() const;                  // Automatically inferred as read-only property "level"
    
    int getHealth() const;                 // Automatically inferred as read-write property "health"
    void setHealth(int health);
    
    // Regular methods - automatically exported
    void attack(const std::string& target);
    bool isAlive() const;
    void levelUp();
    
    // Static methods - automatically exported
    static int getMaxLevel();
    static std::shared_ptr<Player> createDefault();
    
    // Methods you don't want to export - explicitly mark ignore
    EXPORT_LUA_IGNORE()
    void debugInternalState();

private:
    std::string name_;    // Private members automatically ignored
    int level_;
    int health_;
};

// 4. Global functions - automatically infer function names
EXPORT_LUA_FUNCTION()
double calculateDistance(double x1, double y1, double x2, double y2);

EXPORT_LUA_FUNCTION()
int calculateDamage(int attack, int defense);

// 5. Constants - automatically inferred as read-only
EXPORT_LUA_CONSTANT()
static const int MAX_PLAYERS = 100;

EXPORT_LUA_CONSTANT()
static const double PI = 3.14159;

} // namespace game
```

### Advanced Features Usage Example

```cpp
#include "export_macros.h"

EXPORT_LUA_MODULE(AdvancedFeatures)

namespace advanced {

// 1. Singleton class - automatically recognize singleton pattern
class EXPORT_LUA_SINGLETON() GameManager {
public:
    static GameManager& getInstance();     // Automatically recognized as singleton access method
    
    void startGame();
    void stopGame();
    bool isGameRunning() const;
    
    void addPlayer(std::shared_ptr<game::Player> player);
    std::vector<std::shared_ptr<game::Player>> getAllPlayers() const;

private:
    GameManager() = default;               // Private constructor automatically ignored
    bool game_running_ = false;
    std::vector<std::shared_ptr<game::Player>> players_;
};

// 2. Static utility class - only export static members
class EXPORT_LUA_STATIC_CLASS() MathUtils {
public:
    static double clamp(double value, double min, double max);
    static double lerp(double a, double b, double t);
    static int random(int min, int max);
    static double randomFloat(double min, double max);
    
    // Static constants
    static const double PI;
    static const double E;
};

// 3. Abstract base class handling - Smart inheritance mechanism
// Note: Abstract classes are usually not directly exported to Lua, but through
// smart inheritance mechanism that automatically adds base class public methods
// to derived class bindings
class Component {  // No EXPORT_LUA_ABSTRACT_CLASS used
public:
    Component() = default;
    virtual ~Component() = default;
    
    // Pure virtual functions - not directly bound to Lua
    virtual void initialize() = 0;
    virtual void update(double deltaTime) = 0;
    virtual void destroy() = 0;
    
    // Regular virtual functions - automatically inherited to derived class bindings
    virtual bool isActive() const;
    virtual void setActive(bool active);

protected:
    bool active_ = true;
};

// Derived class - automatically includes base class public methods
class EXPORT_LUA_CLASS() ConcreteComponent : public Component {
public:
    ConcreteComponent();
    
    // Implement pure virtual functions
    void initialize() override;
    void update(double deltaTime) override;
    void destroy() override;
    
    // Base class isActive() and setActive() methods will be automatically
    // added to this class's bindings
};

// 4. Operator overloading - automatically map to Lua metamethods
class EXPORT_LUA_CLASS() Vector2D {
public:
    Vector2D();
    Vector2D(double x, double y);
    
    // Property access
    double getX() const;
    void setX(double x);
    double getY() const;
    void setY(double y);
    
    // Arithmetic operators - automatically map to Lua metamethods
    EXPORT_LUA_OPERATOR(+)
    Vector2D operator+(const Vector2D& other) const;    // Map to __add
    
    EXPORT_LUA_OPERATOR(-)
    Vector2D operator-(const Vector2D& other) const;    // Map to __sub
    
    EXPORT_LUA_OPERATOR(*)
    Vector2D operator*(double scalar) const;            // Map to __mul
    
    // Comparison operators
    EXPORT_LUA_OPERATOR(==)
    bool operator==(const Vector2D& other) const;       // Map to __eq
    
    // Subscript operator
    EXPORT_LUA_OPERATOR([])
    double operator[](int index) const;                 // Map to __index

private:
    double x_, y_;
};

} // namespace advanced

// 5. STL container export - automatically generate complete bindings
EXPORT_LUA_VECTOR(int)
EXPORT_LUA_VECTOR(std::string)
EXPORT_LUA_VECTOR(std::shared_ptr<game::Player>)
EXPORT_LUA_MAP(std::string, int)
EXPORT_LUA_MAP(int, std::shared_ptr<game::Player>)

namespace events {

// 6. Event system - automatically infer callback functions
class EXPORT_LUA_CLASS() EventSystem {
public:
    EventSystem();
    
    // Callback functions - automatically infer parameter types and count
    EXPORT_LUA_CALLBACK()
    std::function<void()> OnGameStart;                   // No parameter callback
    
    EXPORT_LUA_CALLBACK()
    std::function<void(std::shared_ptr<game::Player>)> OnPlayerJoin;  // Single parameter callback
    
    EXPORT_LUA_CALLBACK()
    std::function<bool(const std::string&, double)> OnValidateAction;  // Return value callback
    
    // Event trigger methods
    void triggerGameStart();
    void triggerPlayerJoin(std::shared_ptr<game::Player> player);
    bool validateAction(const std::string& action, double value);

private:
    bool initialized_ = false;
};

} // namespace events
```

## Build and Usage

### 1. System Requirements

- **C++17** compatible compiler (GCC 7+, Clang 7+, MSVC 2019+)
- **CMake 3.16+**
- **Operating Systems**: Linux, macOS, Windows

The project is self-contained with all necessary third-party library source code, no additional system dependencies required.

### 2. Build Tool

```bash
# Build from project root directory
mkdir build && cd build
cmake ..
make

# Or use automation script
./scripts/build_and_test_all.sh
```

### 3. Using Automation Scripts

The automation scripts will perform the following operations:
1. Build the lua_binding_generator tool
2. Generate Lua bindings for the complete_test example
3. Compile the complete test project (including runtime library)
4. Run the comprehensive test suite to verify all functionality

#### Unix/Linux/macOS

```bash
# Complete build and test workflow
./scripts/build_and_test_all.sh

# With clean option
./scripts/build_and_test_all.sh --clean

# Clean third-party library build artifacts (free disk space)
./scripts/build_and_test_all.sh --clean-thirdparty

# Complete third-party library cleanup (including executables)
./scripts/build_and_test_all.sh --clean-thirdparty-full

# Verbose output
./scripts/build_and_test_all.sh --verbose

# Show help
./scripts/build_and_test_all.sh --help
```

#### Windows

```cmd
# Complete build and test workflow
scripts\build_and_test_all.bat

# With clean option
scripts\build_and_test_all.bat /clean

# Verbose output
scripts\build_and_test_all.bat /verbose

# Show help
scripts\build_and_test_all.bat /help
```

### 4. Third-party Library Cleanup Tool

The project provides dedicated scripts to clean third-party library build artifacts, helping save disk space:

```bash
# Light cleanup (CMake cache, temporary files)
./scripts/clean_thirdparty.sh

# Complete cleanup (all build artifacts)
./scripts/clean_thirdparty.sh --level=full

# Preview cleanup content (without actually deleting)
./scripts/clean_thirdparty.sh --dry-run

# Clean specific library
./scripts/clean_thirdparty.sh --library=llvm

# Create backup
./scripts/clean_thirdparty.sh --level=full --backup
```

### 5. Generate Lua Bindings

```bash
# Simplest form - generate bindings for complete test example
./lua_binding_generator examples/complete_test/headers/macro_coverage.h

# Process multiple files
./lua_binding_generator examples/complete_test/headers/*.h

# Specify output directory and module name
./lua_binding_generator --output-dir=generated_bindings --module-name=CompleteTestBindings examples/complete_test/headers/*.h

# Enable verbose output
./lua_binding_generator --verbose examples/complete_test/headers/*.h

# Force regeneration of all files
./lua_binding_generator --force-rebuild examples/complete_test/headers/*.h
```

### 6. Run Complete Test Example

```bash
# Build and run complete test suite
cd examples/complete_test/build
./CompleteTestProgram

# Or run from project root directory
examples/complete_test/build/CompleteTestProgram

# Run specific test categories
examples/complete_test/build/CompleteTestProgram --macro-only        # Run only macro tests
examples/complete_test/build/CompleteTestProgram --runtime-only      # Run only runtime tests
examples/complete_test/build/CompleteTestProgram --performance-only  # Run only performance tests
examples/complete_test/build/CompleteTestProgram --stress            # Include stress tests
```

### 7. Command Line Options

| Option | Default | Description |
|--------|---------|-------------|
| `--output-dir` | `generated_bindings` | Output directory |
| `--module-name` | Auto-inferred | Lua module name |
| `--incremental` | `true` | Enable incremental compilation |
| `--force-rebuild` | `false` | Force regeneration of all files |
| `--parallel` | `true` | Enable parallel processing |
| `--max-threads` | `0` (auto) | Maximum number of threads |
| `--verbose` | `false` | Verbose output |
| `--show-stats` | `false` | Show statistics |

## Automatic Inference Features

### Namespace Inference

Priority: C++ namespace > EXPORT_LUA_MODULE > Global

```cpp
EXPORT_LUA_MODULE(GameCore)

namespace game {
    class EXPORT_LUA_CLASS() Player {};  // Inferred as game.Player
}
```

### Smart Inheritance Method Extraction

When a derived class is exported to Lua but its base class is not exported, the tool automatically adds the base class's public methods to the derived class bindings:

```cpp
// Base class not exported
class Entity {
public:
    int getId() const;
    void setId(int id);
    std::string getName() const;
    void setName(const std::string& name);
};

// Derived class exported - automatically includes base class methods
class EXPORT_LUA_CLASS() Player : public Entity {
public:
    void levelUp();
    // getId, setId, getName, setName will be automatically added to bindings
};
```

### Property Inference

Automatically recognize get/set method pairs:

```cpp
class EXPORT_LUA_CLASS() Player {
public:
    int getHealth() const;      // Inferred as read-only property "health"
    void setHealth(int health);
    
    int getLevel() const;       // Inferred as read-only property "level"
    
    double getMana() const;     // Inferred as read-write property "mana"
    void setMana(double mana);
};
```

### Enum Value Inference

```cpp
enum class EXPORT_LUA_ENUM() Status {
    ACTIVE = 1,     // Automatically exported as Status.ACTIVE = 1
    INACTIVE = 2,   // Automatically exported as Status.INACTIVE = 2
    PENDING         // Automatically inferred value as 3
};
```

### Singleton Pattern Detection

Automatically recognize common singleton access methods:

```cpp
class EXPORT_LUA_SINGLETON() GameManager {
public:
    static GameManager& getInstance();  // Automatically recognized
    static GameManager& instance();     // Or this one
    static GameManager* get();          // Or this one
};
```

## Generated Binding Code Example

For the above `Player` class, the tool will generate Sol2 binding code like this:

```cpp
#include <sol/sol.hpp>
#include "complete_example.h"

void register_GameCore_bindings(sol::state& lua) {
    // Create namespace
    auto game_ns = lua["game"].get_or_create<sol::table>();
    
    // Register PlayerStatus enum
    game_ns.new_enum<game::PlayerStatus>("PlayerStatus",
        "ALIVE", game::PlayerStatus::ALIVE,
        "DEAD", game::PlayerStatus::DEAD,
        "RESPAWNING", game::PlayerStatus::RESPAWNING
    );
    
    // Register Player class
    auto player_type = game_ns.new_usertype<game::Player>("Player",
        // Constructors
        sol::constructors<game::Player(), game::Player(const std::string&, int)>(),
        
        // Properties (automatically recognized get/set pairs)
        "name", sol::property(&game::Player::getName, &game::Player::setName),
        "level", sol::readonly_property(&game::Player::getLevel),
        "health", sol::property(&game::Player::getHealth, &game::Player::setHealth),
        
        // Inherited methods from base class (automatically included)
        "getId", &game::Player::getId,
        "setId", &game::Player::setId,
        
        // Methods
        "attack", &game::Player::attack,
        "isAlive", &game::Player::isAlive,
        "levelUp", &game::Player::levelUp,
        
        // Static methods
        "getMaxLevel", &game::Player::getMaxLevel,
        "createDefault", &game::Player::createDefault
    );
    
    // Register global functions to game namespace
    game_ns.set_function("calculateDistance", &game::calculateDistance);
    game_ns.set_function("calculateDamage", &game::calculateDamage);
    
    // Register constants
    game_ns["MAX_PLAYERS"] = game::MAX_PLAYERS;
    game_ns["PI"] = game::PI;
}
```

## Project Structure

```
lua_binding_generator/
├── CMakeLists.txt                      # Main build configuration
├── README.md                           # Project documentation (Chinese)
├── README_EN.md                        # Project documentation (English)
├── include/                            # Header files directory
│   ├── framework/                      # Runtime library headers
│   │   ├── export_macros.h             # Smart inference macro definitions
│   │   ├── lua_runtime_manager.h       # Lua runtime manager
│   │   ├── memory_allocator.h          # Memory allocator
│   │   ├── hot_reload.h                # Hot reload system
│   │   ├── result.h                    # Result type
│   │   └── runtime_logger.h            # Runtime logger
│   └── tools/                          # Tool headers
│       ├── ast_visitor.h               # AST visitor
│       ├── direct_binding_generator.h  # Hard-coded binding generator
│       ├── smart_inference_engine.h    # Smart inference engine
│       ├── incremental_generator.h     # Incremental compilation system
│       ├── compiler_detector.h         # Compiler detection
│       └── logger.h                    # Logging system
├── src/                                # Source files directory
│   ├── framework/                      # Runtime library sources
│   │   ├── lua_runtime_manager.cpp     # Lua runtime manager implementation
│   │   ├── platform_file_watcher.cpp   # File watcher implementation
│   │   └── runtime_logger.cpp          # Runtime logger implementation
│   └── tools/                          # Tool sources
│       ├── main.cpp                    # Tool main program entry
│       ├── ast_visitor.cpp             # AST visitor implementation
│       ├── direct_binding_generator.cpp # Binding generator implementation
│       ├── smart_inference_engine.cpp  # Smart inference engine implementation
│       ├── incremental_generator.cpp   # Incremental compilation implementation
│       ├── compiler_detector.cpp       # Compiler detection implementation
│       └── logger.cpp                  # Logging system implementation
├── examples/                           # Example code
│   ├── CMakeLists.txt                  # Example project build configuration
│   └── complete_test/                  # Complete test example
│       ├── CMakeLists.txt              # Test project build configuration
│       ├── README.md                   # Test project documentation
│       ├── headers/                    # Test headers
│       │   ├── macro_coverage.h        # Macro coverage test
│       │   └── runtime_features.h      # Runtime features test
│       ├── src/                        # Test sources
│       │   ├── main.cpp                # Main test program
│       │   ├── macro_coverage.cpp      # Macro coverage test implementation
│       │   └── runtime_features.cpp    # Runtime features implementation
│       ├── lua_scripts/               # Lua test scripts
│       │   ├── main_test.lua           # Main test script
│       │   ├── macro_test.lua          # Macro test script
│       │   └── class_interaction_test.lua # Class interaction test
│       ├── test_scripts/              # Test script collection
│       │   ├── run_all_tests.lua       # Run all tests
│       │   ├── basic_macro_test.lua    # Basic macro test
│       │   ├── class_binding_test.lua  # Class binding test
│       │   └── runtime_integration_test.lua # Runtime integration test
│       ├── generated_bindings/         # Generated binding files
│       │   ├── CompleteTestBindings_bindings.cpp
│       │   └── CompleteTestBindings_bindings.h
│       └── build/                      # Test build directory
├── scripts/                            # Automation scripts
│   ├── build_and_test_all.sh           # Unix/Linux/macOS build script
│   ├── build_and_test_all.bat          # Windows build script
│   ├── clean_thirdparty.sh             # Unix third-party cleanup script
│   ├── clean_thirdparty.bat            # Windows third-party cleanup script
│   └── README.md                       # Script usage documentation
├── docs/                               # Documentation directory
│   ├── framework/                      # Runtime library documentation
│   │   ├── API_REFERENCE.md            # API reference
│   │   └── QUICK_START.md              # Quick start guide
│   └── tools/                          # Tool documentation
│       ├── BUILD_GUIDE.md              # Build guide
│       ├── EXAMPLES.md                 # Examples documentation
│       └── USAGE.md                    # Usage guide
├── cmake/                              # CMake configuration files
│   ├── LuaBindingRuntimeConfig.cmake.in
│   └── lua_binding_runtime.pc.in
├── generated_bindings/                 # Generated binding files (created at runtime)
├── build/                              # Build directory (created at runtime)
└── thirdparty/                         # Third-party libraries (self-contained)
    ├── llvm-20.1.8/                    # LLVM compiler infrastructure
    ├── clang-tools-extra-20.1.8.src/   # Clang tools extra
    ├── lua-5.4.8/                      # Lua interpreter
    ├── sol2-3.3.0/                     # Sol2 C++ Lua binding library
    ├── spdlog-1.15.3/                  # High-performance logging library
    └── zstd-1.5.7/                     # Compression library
```


## Performance Optimization

### Incremental Compilation

- Smart caching based on file content hashes
- Dependency tracking and propagation
- Can significantly reduce regeneration time in large projects

### Parallel Processing

- Multi-threaded parallel analysis and generation
- Smart task allocation
- Support for fast processing of large projects

### Memory Optimization

- Remove template parsing overhead
- Direct string operations
- Optimized AST traversal

## Troubleshooting

### Common Issues

1. **Compiler doesn't support `__attribute__((annotate))`**
   - Use Clang compiler
   - Or upgrade to GCC version that supports this feature

2. **Export items not found**
   - Make sure you used `EXPORT_LUA_*` macros
   - Check if macros are in correct positions

3. **Incremental compilation not working**
   - Delete cache file and retry: `rm .lua_binding_cache`
   - Use `--force-rebuild` to force regeneration

4. **Generated binding compilation fails**
   - Check Sol2 version compatibility
   - Look at error messages in generated code

5. **Third-party libraries taking too much disk space**
   - Use cleanup script: `./scripts/clean_thirdparty.sh --level=full`
   - Regularly clean build artifacts

6. **Runtime library related errors**
   - Ensure correct runtime library headers are included: `#include "lua_runtime_manager.h"`
   - Check memory allocator configuration
   - View test report: `examples/complete_test/build/test_report.txt`

### Debug Options

```bash
# Enable verbose output
./lua_binding_generator --verbose --show-stats examples/*.h

# Force regeneration (ignore cache)
./lua_binding_generator --force-rebuild examples/*.h

# Disable parallel processing (for debugging)
./lua_binding_generator --parallel=false examples/*.h
```

## Summary

lua_binding_generator achieves the following key goals:

1. **Zero Configuration Usage** - Most scenarios need only parameter-free macros
2. **Smart Inference** - Automatically infer most configuration information from AST  
3. **Full Coverage** - Support all modern C++ features
4. **High Performance** - Hard-coded generator, efficient incremental compilation
5. **Completely Self-Contained** - Built-in all dependencies, simplified deployment
6. **Cross-Platform Support** - Full platform support for Linux, macOS, Windows

lua_binding_generator truly achieves the design goal of "minimizing user mental burden and maximizing work efficiency".

## Contributing

Welcome to submit Issues and Pull Requests!

## License

This project uses the MIT License.

---

📖 **Documentation**: [English](./README_EN.md) | [中文](./README.md)