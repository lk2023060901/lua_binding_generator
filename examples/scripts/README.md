# Lua Test Scripts

This directory contains comprehensive test scripts for validating the generated Lua bindings.

## Test Files

### Core Test Scripts

1. **`test_simple.lua`** - Tests for the simple example
   - Basic functions, classes, and constants
   - Simple Calculator class functionality
   - DataContainer operations
   - StringUtils static methods
   - Basic error handling

2. **`test_game_engine.lua`** - Tests for the game engine example
   - Vector2 math operations and operator overloading
   - GameObject hierarchy and inheritance
   - Time management system
   - Collision detection utilities
   - Resource management
   - Singleton pattern (World, GameManager)
   - Event system with callbacks

3. **`test_comprehensive.lua`** - Tests for comprehensive_test.h features
   - Advanced inheritance scenarios
   - Complex callback systems
   - STL container bindings
   - Smart pointer integration
   - Template class instances
   - Multi-parameter events
   - Abstract base classes

### Integration & Utility Scripts

4. **`test_bindings_integration.lua`** - Integration testing
   - Memory management validation
   - Exception handling verification
   - Type safety checks
   - Performance basic benchmarks
   - Thread safety simulation
   - Cross-module compatibility

5. **`run_all_tests.lua`** - Test suite runner
   - Executes all test scripts
   - Provides comprehensive reporting
   - Color-coded output (if supported)
   - Summary statistics and recommendations

## Usage

### Testing the Framework Itself (No Bindings Required)

```bash  
# Verify that the testing framework works correctly
lua scripts/test_framework_standalone.lua

# Safely run all tests with module availability checking
lua scripts/run_tests_safe.lua
```

### Running Individual Tests (Requires Generated Bindings)

```bash
# Test simple example bindings
lua scripts/test_simple.lua

# Test game engine bindings  
lua scripts/test_game_engine.lua

# Test comprehensive features
lua scripts/test_comprehensive.lua

# Test integration aspects
lua scripts/test_bindings_integration.lua
```

### Running All Tests (Requires Generated Bindings)

```bash
# Run complete test suite
lua scripts/run_all_tests.lua
```

**Note**: All test scripts now include module availability checking and will gracefully skip tests if the required bindings are not loaded.

## Prerequisites

Before running these tests, ensure:

1. **Generated Bindings**: The lua_binding_generator has processed your header files
   ```bash
   lua_binding_generator --output_dir=generated_bindings examples/*.h
   ```

2. **Lua Integration**: Your application properly loads the generated bindings
   - Sol2 library integrated
   - Binding registration functions called
   - Lua state properly initialized

3. **Module Loading**: The test assumes modules are available as:
   - `simple.*` - For simple_example.h exports
   - `engine.*` - For game_engine.h exports  
   - `game.*` - For comprehensive_test.h exports
   - `operators.*` - For operator overloading tests

## Expected Output

### Successful Test Run
```
=== Simple Example Lua Binding Tests ===

--- Testing Enums ---
✓ PASS: Color.RED value
✓ PASS: Color.GREEN value
...

--- Testing Calculator Class ---
✓ PASS: Calculator default constructor
✓ PASS: Calculator add method
...

=== Test Summary ===
Tests passed: 45
Tests failed: 0
Total tests: 45
🎉 All tests passed!
```

### Failed Test Example
```
--- Testing Calculator Class ---
✓ PASS: Calculator default constructor
✗ FAIL: Calculator add method - Expected: 8.0, Got: 5.0
...

=== Test Summary ===
Tests passed: 44
Tests failed: 1
Total tests: 45
❌ Some tests failed!
```

## Test Categories

### 1. Basic Functionality
- Function calls with various parameter types
- Property get/set operations
- Constant value access
- Enum value verification

### 2. Object Oriented Features
- Constructor variations
- Method calls on instances
- Static method access
- Inheritance behavior
- Virtual function dispatch

### 3. Advanced Features
- Operator overloading (`+`, `-`, `*`, `/`, `==`, `[]`, etc.)
- Callback function registration and triggering
- Smart pointer handling
- STL container integration
- Template class instances

### 4. Memory Management
- Object lifecycle management
- Reference counting behavior
- Garbage collection interaction
- Memory leak prevention

### 5. Error Handling
- Exception safety
- Invalid parameter handling
- Null pointer protection
- Boundary condition testing

## Customization

### Adding New Tests

To add tests for your own classes:

1. Create a new test file following the pattern:
   ```lua
   -- test_my_module.lua
   local tests_passed = 0
   local tests_failed = 0
   
   function assert_equal(actual, expected, message)
       -- ... test helper implementation
   end
   
   -- Your tests here
   assert_equal(my_module.my_function(5), 10, "My function test")
   ```

2. Add it to `run_all_tests.lua`:
   ```lua
   local test_files = {
       -- existing files...
       {
           file = script_dir .. "test_my_module.lua",
           description = "My Module Tests", 
           required = true
       }
   }
   ```

### Modifying Test Assertions

The test framework provides these assertion functions:
- `assert_equal(actual, expected, message)` - Exact equality
- `assert_true(condition, message)` - Boolean true
- `assert_false(condition, message)` - Boolean false  
- `assert_near(actual, expected, tolerance, message)` - Floating point comparison
- `safe_call(func, message)` - Exception-safe function call

## Troubleshooting

### Common Issues

1. **Module Not Found**
   ```
   Error: attempt to index global 'simple' (a nil value)
   ```
   - Ensure bindings are generated and registered
   - Check module name matches EXPORT_LUA_MODULE() declaration

2. **Function Not Available**
   ```
   Error: attempt to call field 'myFunction' (a nil value)
   ```
   - Verify function has EXPORT_LUA_FUNCTION() macro
   - Check for C++ compilation errors
   - Ensure binding registration is complete

3. **Type Mismatch**
   ```
   FAIL: Expected: 5, Got: userdata
   ```
   - Check return type handling in bindings
   - Verify Sol2 type conversion setup
   - Review custom type registrations

4. **Memory Issues**
   ```
   Segmentation fault during test
   ```
   - Check object lifetime management
   - Verify shared_ptr usage in C++
   - Review callback function storage

### Debug Mode

Enable verbose output by setting debug flags in test files:
```lua
local DEBUG = true

function debug_print(msg)
    if DEBUG then
        print("[DEBUG] " .. msg)
    end
end
```

## Integration with CI/CD

These scripts can be integrated into continuous integration:

```bash
#!/bin/bash
# CI test script

# Generate bindings
./lua_binding_generator examples/*.h || exit 1

# Build project with bindings
make || exit 1

# Run Lua tests
lua examples/scripts/run_all_tests.lua || exit 1

echo "All tests passed!"
```

For more information about the lua_binding_generator tool, see the main README.md in the project root.