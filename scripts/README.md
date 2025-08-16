# Lua Binding Generator - Build and Test Scripts

This directory contains automated scripts for building and testing the Lua Binding Generator project.

## Scripts

### build_and_test_all.sh (Linux/macOS)
Complete automation script for Unix-like systems.

```bash
# Basic usage
./scripts/build_and_test_all.sh

# Clean build before running
./scripts/build_and_test_all.sh --clean

# Verbose output
./scripts/build_and_test_all.sh --verbose

# Show help
./scripts/build_and_test_all.sh --help
```

### build_and_test_all.bat (Windows)
Complete automation script for Windows systems.

```cmd
# Basic usage
scripts\build_and_test_all.bat

# Clean build before running
scripts\build_and_test_all.bat --clean

# Verbose output
scripts\build_and_test_all.bat --verbose

# Show help
scripts\build_and_test_all.bat --help
```

## What the Scripts Do

1. **Check Prerequisites**
   - Verify CMake is installed
   - Verify build tools (make/MSBuild) are available
   - Confirm project structure

2. **Build lua_binding_generator Tool**
   - Configure project with CMake
   - Compile the binding generator executable
   - Verify successful build

3. **Generate Lua Bindings**
   - Find all header files in examples directory
   - Run lua_binding_generator on all headers
   - Generate Sol2 binding files

4. **Build Example Programs**
   - Compile all example programs with generated bindings
   - Link with Lua libraries
   - Verify successful compilation

5. **Run Tests**
   - Execute all built example programs
   - Capture output and verify execution
   - Report success/failure statistics

6. **Show Summary**
   - Display build and test results
   - List generated files and executables
   - Provide next steps

## Options

- `--clean`: Remove build and generated directories before starting
- `--verbose`: Show detailed output from all build steps
- `--help`: Display usage information

## Requirements

### Linux/macOS
- CMake 3.16+
- Make or Ninja
- C++17 compatible compiler (GCC 7+, Clang 7+)
- Lua development libraries

### Windows
- CMake 3.16+
- Visual Studio 2019+ or MinGW-w64
- C++17 compatible compiler
- Lua development libraries

## Example Output

```
=== Lua Binding Generator - Complete Build and Test Workflow ===
Starting automated build and test process...

=== Checking Prerequisites ===
✓ All prerequisites satisfied

=== Building Lua Binding Generator Tool ===
ℹ Configuring project with CMake...
ℹ Building lua_binding_generator...
✓ Lua binding generator built successfully

=== Generating Lua Bindings for All Examples ===
ℹ Found header files:
  - simple_example.h
  - game_engine.h
  - comprehensive_test.h
ℹ Running lua_binding_generator...
✓ Generated binding files:
  - simple_bindings.cpp
  - engine_bindings.cpp
  - game_bindings.cpp

=== Building All Example Programs ===
ℹ Compiling all examples...
✓ Successfully built example programs:
  - simple_example
  - game_engine_example
  - comprehensive_test

=== Running Example Programs ===
ℹ Running simple_example...
✓ simple_example completed successfully
ℹ Running game_engine_example...
✓ game_engine_example completed successfully
ℹ Running comprehensive_test...
✓ comprehensive_test completed successfully

=== Test Results Summary ===
✓ Successful executions: 3
✓ Failed executions: 0

=== Build and Test Summary ===
Project: Lua Binding Generator
Generated binding files: 3
Built executable programs: 3

✓ Complete workflow finished successfully!

You can now:
  - Run individual examples from the build directory
  - Create Lua test scripts to verify the bindings
  - Extend the examples with more C++ features
```

## Troubleshooting

### Common Issues

1. **CMake not found**
   - Install CMake 3.16 or later
   - Ensure CMake is in system PATH

2. **Build tools not found**
   - Linux/macOS: Install build-essential or Xcode Command Line Tools
   - Windows: Install Visual Studio with C++ workload

3. **Lua libraries not found**
   - Linux: `sudo apt-get install liblua5.4-dev` (Ubuntu/Debian)
   - macOS: `brew install lua` (Homebrew)
   - Windows: Use vcpkg or manual installation

4. **Compilation errors**
   - Ensure C++17 compatible compiler
   - Check all dependencies are properly installed
   - Try `--clean` option to rebuild from scratch

### Getting Help

Run the script with `--help` option for usage information:

```bash
./scripts/build_and_test_all.sh --help
```

For more detailed debugging, use the `--verbose` option:

```bash
./scripts/build_and_test_all.sh --verbose
```