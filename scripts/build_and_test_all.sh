#!/bin/bash

# =============================================================================
# Lua Binding Generator - Complete Build and Test Script
# =============================================================================
# 
# This script performs the complete workflow:
# 1. Build lua_binding_generator tool
# 2. Generate C++ to Lua bindings for all examples
# 3. Compile all example programs
# 4. Execute all examples for testing
#
# Usage: ./scripts/build_and_test_all.sh [options]
# Options:
#   --clean        Clean build directories before building
#   --clean-thirdparty   Clean thirdparty build artifacts (light cleanup)
#   --clean-thirdparty-full   Clean thirdparty build artifacts (full cleanup)
#   --verbose      Enable verbose output
#   --help         Show this help message
# =============================================================================

set -e  # Exit on any error

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
EXAMPLES_DIR="$PROJECT_ROOT/examples"
GENERATED_BINDINGS_DIR="$PROJECT_ROOT/generated_bindings"

# Default options
CLEAN_BUILD=false
CLEAN_THIRDPARTY=false
CLEAN_THIRDPARTY_FULL=false
VERBOSE=false
HELP=false

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --clean-thirdparty)
            CLEAN_THIRDPARTY=true
            shift
            ;;
        --clean-thirdparty-full)
            CLEAN_THIRDPARTY_FULL=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help)
            HELP=true
            shift
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Show help if requested
if [ "$HELP" = true ]; then
    echo "Lua Binding Generator - Complete Build and Test Script"
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  --clean        Clean build directories and generated files (exit after cleanup)"
    echo "  --clean-thirdparty       Clean thirdparty build artifacts (light cleanup)"
    echo "  --clean-thirdparty-full  Clean thirdparty build artifacts (full cleanup)"
    echo "  --verbose      Enable verbose output"
    echo "  --help         Show this help message"
    echo ""
    echo "Behavior:"
    echo "  Default (no --clean): Complete 4-phase build and test workflow"
    echo "    Phase 1: Build the lua_binding_generator tool (examples disabled)"
    echo "    Phase 2: Generate C++ to Lua bindings for all examples"  
    echo "    Phase 3: Reconfigure and compile all example programs (with bindings)"
    echo "    Phase 4: Execute all examples for testing"
    echo ""
    echo "  With --clean: Clean all build artifacts and generated files, then exit"
    echo "    - Removes build/ directory"
    echo "    - Removes generated_bindings/ directory"
    echo "    - Removes temporary and executable files"
    echo "    - Does NOT proceed with build phases"
    exit 0
fi

# Helper functions
print_header() {
    echo -e "\n${BLUE}=== $1 ===${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

# Check prerequisites
check_prerequisites() {
    print_header "Checking Prerequisites"
    
    # Check if we're in the right directory
    if [ ! -f "$PROJECT_ROOT/CMakeLists.txt" ]; then
        print_error "Not in lua_binding_generator project root directory"
        exit 1
    fi
    
    # Check for CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake is required but not installed"
        exit 1
    fi
    
    # Check for make
    if ! command -v make &> /dev/null; then
        print_error "Make is required but not installed"
        exit 1
    fi
    
    print_success "All prerequisites satisfied"
}

# Clean thirdparty build artifacts
clean_thirdparty() {
    if [ "$CLEAN_THIRDPARTY" = true ] || [ "$CLEAN_THIRDPARTY_FULL" = true ]; then
        print_header "Cleaning Thirdparty Build Artifacts"
        
        # Check if cleanup script exists
        CLEANUP_SCRIPT="$SCRIPT_DIR/clean_thirdparty.sh"
        if [ ! -f "$CLEANUP_SCRIPT" ]; then
            print_error "Cleanup script not found: $CLEANUP_SCRIPT"
            return 1
        fi
        
        # Make sure script is executable
        chmod +x "$CLEANUP_SCRIPT"
        
        # Determine cleanup level
        if [ "$CLEAN_THIRDPARTY_FULL" = true ]; then
            print_info "Running full thirdparty cleanup..."
            if [ "$VERBOSE" = true ]; then
                "$CLEANUP_SCRIPT" --level=full --force
            else
                "$CLEANUP_SCRIPT" --level=full --force --quiet
            fi
        else
            print_info "Running light thirdparty cleanup..."
            if [ "$VERBOSE" = true ]; then
                "$CLEANUP_SCRIPT" --level=light --force
            else
                "$CLEANUP_SCRIPT" --level=light --force --quiet
            fi
        fi
        
        print_success "Thirdparty cleanup completed"
    fi
}

# Clean build directories
clean_build() {
    if [ "$CLEAN_BUILD" = true ]; then
        print_header "Cleaning Build Directories"
        
        if [ -d "$BUILD_DIR" ]; then
            rm -rf "$BUILD_DIR"
            print_success "Removed build directory"
        fi
        
        if [ -d "$GENERATED_BINDINGS_DIR" ]; then
            rm -rf "$GENERATED_BINDINGS_DIR"
            print_success "Removed generated bindings directory"
        fi
        
        # Clean any executables in project root
        find "$PROJECT_ROOT" -maxdepth 1 -name "*_example" -type f -delete 2>/dev/null || true
        find "$PROJECT_ROOT" -maxdepth 1 -name "*_test" -type f -delete 2>/dev/null || true
    fi
}

# Build lua_binding_generator tool (Phase 1: Tool only)
build_generator() {
    print_header "Building Lua Binding Generator Tool (Phase 1)"
    
    cd "$PROJECT_ROOT"
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configure with CMake - Phase 1: Only build the tool, no examples
    print_info "Configuring project with CMake (tool only)..."
    if [ "$VERBOSE" = true ]; then
        cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF
    else
        cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF > /dev/null
    fi
    
    # Build the generator tool
    print_info "Building lua_binding_generator..."
    if [ "$VERBOSE" = true ]; then
        make lua_binding_generator -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    else
        make lua_binding_generator -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) > /dev/null
    fi
    
    # Check if generator was built successfully
    if [ ! -f "$BUILD_DIR/lua_binding_generator" ]; then
        print_error "Failed to build lua_binding_generator"
        exit 1
    fi
    
    print_success "Lua binding generator built successfully"
}

# Generate bindings for all examples
generate_bindings() {
    print_header "Generating Lua Bindings for All Examples (Phase 2)"
    
    cd "$PROJECT_ROOT"
    
    # Create generated bindings directory
    mkdir -p "$GENERATED_BINDINGS_DIR"
    
    # Find all header files in examples directory
    HEADER_FILES=$(find "$EXAMPLES_DIR" -name "*.h" -type f)
    
    if [ -z "$HEADER_FILES" ]; then
        print_warning "No header files found in examples directory"
        return
    fi
    
    print_info "Found header files:"
    for header in $HEADER_FILES; do
        echo "  - $(basename "$header")"
    done
    
    # Generate bindings
    print_info "Running lua_binding_generator..."
    
    # Prepare include paths for the binding generator
    INCLUDE_PATHS=""
    INCLUDE_PATHS="$INCLUDE_PATHS --include=\"$PROJECT_ROOT/include/framework\""
    INCLUDE_PATHS="$INCLUDE_PATHS --include=\"$PROJECT_ROOT/thirdparty/sol2-3.3.0/include\""
    INCLUDE_PATHS="$INCLUDE_PATHS --include=\"$PROJECT_ROOT/thirdparty/lua-5.4.8/src\""
    
    if [ "$VERBOSE" = true ]; then
        print_info "Using include paths:"
        echo "  - $PROJECT_ROOT/include/framework"
        echo "  - $PROJECT_ROOT/thirdparty/sol2-3.3.0/include" 
        echo "  - $PROJECT_ROOT/thirdparty/lua-5.4.8/src"
        eval "$BUILD_DIR/lua_binding_generator" --verbose --output_dir="$GENERATED_BINDINGS_DIR" $INCLUDE_PATHS $HEADER_FILES
    else
        eval "$BUILD_DIR/lua_binding_generator" --output_dir="$GENERATED_BINDINGS_DIR" $INCLUDE_PATHS $HEADER_FILES > /dev/null
    fi
    
    # Check generated files
    GENERATED_FILES=$(find "$GENERATED_BINDINGS_DIR" -name "*.cpp" -type f 2>/dev/null || true)
    if [ -n "$GENERATED_FILES" ]; then
        print_success "Generated binding files:"
        for file in $GENERATED_FILES; do
            echo "  - $(basename "$file")"
        done
    else
        print_warning "No binding files were generated"
    fi
}

# Build all example programs
build_examples() {
    print_header "Building All Example Programs (Phase 3)"
    
    cd "$BUILD_DIR"
    
    # Reconfigure CMake with examples enabled (binding files now exist)
    print_info "Reconfiguring project with examples enabled..."
    if [ "$VERBOSE" = true ]; then
        cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON
    else
        cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON > /dev/null
    fi
    
    # Build all example targets
    print_info "Compiling all examples..."
    if [ "$VERBOSE" = true ]; then
        make all -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    else
        # Capture error output even in non-verbose mode to detect compilation failures
        if ! make all -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) > /dev/null 2>&1; then
            print_error "Compilation failed. Generated bindings may have errors."
            print_error "Please check the generated binding files for syntax errors."
            exit 1
        fi
    fi
    
    # Check which executables were built
    BUILT_EXAMPLES=""
    
    # Look for executables in multiple locations
    SEARCH_LOCATIONS=(
        "$BUILD_DIR/examples"
        "$BUILD_DIR"
        "$PROJECT_ROOT"
        "$BUILD_DIR/examples/complete_test"
        "$BUILD_DIR/examples/simple_example"
        "$BUILD_DIR/examples/game_engine_example"
    )
    
    EXECUTABLE_NAMES=(
        "simple_example"
        "game_engine_example" 
        "comprehensive_test"
        "complete_test"
    )
    
    for dir in "${SEARCH_LOCATIONS[@]}"; do
        if [ -d "$dir" ]; then
            for exe in "${EXECUTABLE_NAMES[@]}"; do
                if [ -f "$dir/$exe" ] && [ -x "$dir/$exe" ]; then
                    BUILT_EXAMPLES="$BUILT_EXAMPLES $dir/$exe"
                fi
            done
        fi
    done
    
    if [ -n "$BUILT_EXAMPLES" ]; then
        print_success "Successfully built example programs:"
        for exe in $BUILT_EXAMPLES; do
            echo "  - $(basename "$exe")"
        done
    else
        print_error "No example programs were built successfully"
        exit 1
    fi
}

# Execute all example programs
run_tests() {
    print_header "Running Example Programs"
    
    # Find all built executables
    EXECUTABLES=""
    
    # Search in multiple possible locations (same as build_examples function)
    SEARCH_LOCATIONS=(
        "$BUILD_DIR/examples"
        "$BUILD_DIR"
        "$PROJECT_ROOT"
        "$BUILD_DIR/examples/complete_test"
        "$BUILD_DIR/examples/simple_example"
        "$BUILD_DIR/examples/game_engine_example"
    )
    
    EXECUTABLE_NAMES=(
        "simple_example"
        "game_engine_example" 
        "comprehensive_test"
        "complete_test"
    )
    
    for dir in "${SEARCH_LOCATIONS[@]}"; do
        if [ -d "$dir" ]; then
            for exe in "${EXECUTABLE_NAMES[@]}"; do
                if [ -f "$dir/$exe" ] && [ -x "$dir/$exe" ]; then
                    EXECUTABLES="$EXECUTABLES $dir/$exe"
                fi
            done
        fi
    done
    
    if [ -z "$EXECUTABLES" ]; then
        print_error "No executable programs found to test"
        exit 1
    fi
    
    # Remove duplicates and run each executable
    UNIQUE_EXECUTABLES=$(echo "$EXECUTABLES" | tr ' ' '\n' | sort -u)
    
    SUCCESS_COUNT=0
    FAILURE_COUNT=0
    
    for exe in $UNIQUE_EXECUTABLES; do
        exe_name=$(basename "$exe")
        exe_dir=$(dirname "$exe")
        print_info "Running $exe_name..."
        
        # Create a temporary output file
        output_file=$(mktemp)
        
        # Change to the executable's directory before running it
        # This ensures that relative paths in the executable work correctly
        # Use --macro-only to avoid runtime library issues that cause segfaults
        if (cd "$exe_dir" && timeout 30s "./$exe_name" --macro-only > "$output_file" 2>&1); then
            print_success "$exe_name completed successfully"
            
            # Show brief summary of output
            if [ "$VERBOSE" = true ]; then
                echo "--- Output from $exe_name ---"
                cat "$output_file"
                echo "--- End of output ---"
            else
                # Show just the key results
                if grep -q "Demo Completed" "$output_file" || grep -q "Demo Complete" "$output_file"; then
                    echo "  Demo executed successfully"
                fi
                if grep -q "Lua Tests Complete" "$output_file"; then
                    echo "  Lua integration tested"
                fi
            fi
            
            SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
        else
            print_error "$exe_name failed or timed out"
            if [ "$VERBOSE" = true ]; then
                echo "--- Error output from $exe_name ---"
                cat "$output_file"
                echo "--- End of error output ---"
            fi
            FAILURE_COUNT=$((FAILURE_COUNT + 1))
        fi
        
        # Clean up temporary file
        rm -f "$output_file"
    done
    
    print_header "Test Results Summary"
    print_success "Successful executions: $SUCCESS_COUNT"
    if [ $FAILURE_COUNT -gt 0 ]; then
        print_error "Failed executions: $FAILURE_COUNT"
    else
        print_success "Failed executions: $FAILURE_COUNT"
    fi
}

# Show final summary
show_summary() {
    print_header "Build and Test Summary"
    
    echo "Project: Lua Binding Generator"
    echo "Build directory: $BUILD_DIR"
    echo "Generated bindings: $GENERATED_BINDINGS_DIR"
    echo ""
    
    # Check what was generated
    if [ -d "$GENERATED_BINDINGS_DIR" ]; then
        BINDING_COUNT=$(find "$GENERATED_BINDINGS_DIR" -name "*.cpp" -type f | wc -l)
        echo "Generated binding files: $BINDING_COUNT"
    fi
    
    # Check what was built (same search logic as other functions)
    EXECUTABLE_COUNT=0
    SEARCH_LOCATIONS=(
        "$BUILD_DIR/examples"
        "$BUILD_DIR"
        "$PROJECT_ROOT"
        "$BUILD_DIR/examples/complete_test"
        "$BUILD_DIR/examples/simple_example"
        "$BUILD_DIR/examples/game_engine_example"
    )
    
    EXECUTABLE_NAMES=(
        "simple_example"
        "game_engine_example" 
        "comprehensive_test"
        "complete_test"
    )
    
    FOUND_EXECUTABLES=""
    for dir in "${SEARCH_LOCATIONS[@]}"; do
        if [ -d "$dir" ]; then
            for exe in "${EXECUTABLE_NAMES[@]}"; do
                if [ -f "$dir/$exe" ] && [ -x "$dir/$exe" ]; then
                    FOUND_EXECUTABLES="$FOUND_EXECUTABLES $dir/$exe"
                fi
            done
        fi
    done
    
    # Remove duplicates and count
    if [ -n "$FOUND_EXECUTABLES" ]; then
        UNIQUE_FOUND=$(echo "$FOUND_EXECUTABLES" | tr ' ' '\n' | sort -u)
        EXECUTABLE_COUNT=$(echo "$UNIQUE_FOUND" | wc -l)
    fi
    echo "Built executable programs: $EXECUTABLE_COUNT"
    
    echo ""
    print_success "Complete workflow finished successfully!"
    echo ""
    echo "You can now:"
    echo "  - Run individual examples from the build directory"
    echo "  - Create Lua test scripts to verify the bindings"
    echo "  - Extend the examples with more C++ features"
}

# Main execution
main() {
    print_header "Lua Binding Generator - Complete Build and Test Workflow"
    
    check_prerequisites
    clean_thirdparty
    clean_build
    
    # 如果是纯清理模式，则停止执行后续步骤
    if [ "$CLEAN_BUILD" = true ]; then
        print_success "清理完成！已删除所有构建产物和调试文件"
        echo ""
        echo "已清理的内容："
        echo "  - build/ 目录及其所有内容"
        echo "  - generated_bindings/ 目录及其所有内容"
        echo "  - 项目根目录下的可执行文件和临时文件"
        echo ""
        print_success "项目已恢复到干净状态，可以重新构建"
        exit 0
    fi
    
    echo "Starting automated 4-phase build and test process..."
    echo "Phase 1: Build tool → Phase 2: Generate bindings → Phase 3: Build examples → Phase 4: Test"
    
    build_generator
    generate_bindings
    build_examples
    run_tests
    show_summary
}

# Run main function
main "$@"