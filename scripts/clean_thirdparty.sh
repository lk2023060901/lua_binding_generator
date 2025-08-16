#!/bin/bash

# Ensure we're using bash 3.0+ compatible features

# =============================================================================
# Lua Binding Generator - Third-party Build Artifacts Cleanup Script
# =============================================================================
# 
# This script cleans build artifacts from the thirdparty directory to:
# - Free up disk space (thirdparty builds can be several GB)
# - Resolve build cache issues
# - Prepare clean distribution packages
# - Reset third-party libraries to original state
#
# Usage: ./scripts/clean_thirdparty.sh [options]
# Options:
#   --level=<light|full>   Cleaning level (default: light)
#   --library=<name>       Clean specific library only
#   --dry-run              Show what would be deleted without actually deleting
#   --quiet                Suppress output except errors
#   --force                Skip confirmation prompts
#   --backup               Create backup of important build artifacts
#   --help                 Show this help message
# =============================================================================

set -e  # Exit on any error

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
THIRDPARTY_DIR="$PROJECT_ROOT/thirdparty"

# Default options
CLEAN_LEVEL="light"
TARGET_LIBRARY=""
DRY_RUN=false
QUIET=false
FORCE=false
BACKUP=false
HELP=false

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Counters for statistics
DELETED_FILES=0
DELETED_DIRS=0
FREED_SPACE=0

# Function to print colored output
print_info() {
    if [[ "$QUIET" != "true" ]]; then
        echo -e "${BLUE}ℹ${NC} $1"
    fi
}

print_success() {
    if [[ "$QUIET" != "true" ]]; then
        echo -e "${GREEN}✓${NC} $1"
    fi
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1" >&2
}

print_error() {
    echo -e "${RED}✗${NC} $1" >&2
}

print_header() {
    if [[ "$QUIET" != "true" ]]; then
        echo -e "${CYAN}=== $1 ===${NC}"
    fi
}

# Function to show help
show_help() {
    cat << EOF
Lua Binding Generator - Third-party Cleanup Script

USAGE:
    $0 [OPTIONS]

DESCRIPTION:
    Cleans build artifacts from the thirdparty directory. Supports different
    cleaning levels and selective library cleaning.

OPTIONS:
    --level=<light|full>   Cleaning level:
                          light: CMake cache, temp files (default)
                          full:  All build artifacts, restore to source state
    
    --library=<name>       Clean specific library only. Available libraries:
                          llvm, clang-tools-extra, lua, sol2, spdlog, zstd, all
    
    --dry-run              Show what would be deleted without actually deleting
    --quiet                Suppress informational output
    --force                Skip confirmation prompts
    --backup               Create backup of important build artifacts before deletion
    --help                 Show this help message

EXAMPLES:
    $0                     # Light cleanup of all libraries
    $0 --level=full        # Complete cleanup of all libraries
    $0 --library=lua       # Clean only Lua build artifacts
    $0 --dry-run           # Preview what would be cleaned
    $0 --level=full --backup --force  # Full cleanup with backup, no prompts

CLEANING LEVELS:
    light: Removes CMake cache, temporary build files, object files
    full:  Removes all build artifacts including executables and libraries
EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --level=*)
            CLEAN_LEVEL="${1#*=}"
            if [[ "$CLEAN_LEVEL" != "light" && "$CLEAN_LEVEL" != "full" ]]; then
                print_error "Invalid clean level: $CLEAN_LEVEL. Use 'light' or 'full'"
                exit 1
            fi
            shift
            ;;
        --library=*)
            TARGET_LIBRARY="${1#*=}"
            shift
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --quiet)
            QUIET=true
            shift
            ;;
        --force)
            FORCE=true
            shift
            ;;
        --backup)
            BACKUP=true
            shift
            ;;
        --help)
            HELP=true
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Show help if requested
if [[ "$HELP" == "true" ]]; then
    show_help
    exit 0
fi

# Function to get file size
get_file_size() {
    if [[ -f "$1" ]]; then
        if command -v stat >/dev/null 2>&1; then
            # Try different stat formats (macOS vs Linux)
            stat -f%z "$1" 2>/dev/null || stat -c%s "$1" 2>/dev/null || echo 0
        else
            echo 0
        fi
    else
        echo 0
    fi
}

# Function to format file size
format_size() {
    local size=$1
    if [[ $size -lt 1024 ]]; then
        echo "${size}B"
    elif [[ $size -lt 1048576 ]]; then
        echo "$((size / 1024))KB"
    elif [[ $size -lt 1073741824 ]]; then
        echo "$((size / 1048576))MB"
    else
        echo "$((size / 1073741824))GB"
    fi
}

# Function to safely remove file or directory
safe_remove() {
    local target="$1"
    local description="$2"
    
    if [[ ! -e "$target" ]]; then
        return 0
    fi
    
    local size=0
    if [[ -f "$target" ]]; then
        size=$(get_file_size "$target")
        DELETED_FILES=$((DELETED_FILES + 1))
    elif [[ -d "$target" ]]; then
        size=$(du -s "$target" 2>/dev/null | cut -f1 || echo 0)
        size=$((size * 1024))  # Convert from KB to bytes
        DELETED_DIRS=$((DELETED_DIRS + 1))
    fi
    
    FREED_SPACE=$((FREED_SPACE + size))
    
    if [[ "$DRY_RUN" == "true" ]]; then
        print_info "[DRY RUN] Would remove $description: $target ($(format_size $size))"
        return 0
    fi
    
    if [[ "$BACKUP" == "true" && -e "$target" ]]; then
        local backup_dir="$PROJECT_ROOT/backup_$(date +%Y%m%d_%H%M%S)"
        local backup_target="$backup_dir/thirdparty/$(basename "$target")"
        
        print_info "Creating backup: $backup_target"
        mkdir -p "$(dirname "$backup_target")"
        cp -r "$target" "$backup_target" 2>/dev/null || true
    fi
    
    print_info "Removing $description: $(basename "$target") ($(format_size $size))"
    rm -rf "$target"
}

# Function to clean CMake artifacts
clean_cmake_artifacts() {
    local lib_dir="$1"
    local lib_name="$2"
    
    print_info "Cleaning CMake artifacts in $lib_name..."
    
    # CMake cache and configuration files
    safe_remove "$lib_dir/CMakeCache.txt" "CMake cache"
    safe_remove "$lib_dir/CMakeFiles" "CMake files directory"
    safe_remove "$lib_dir/cmake_install.cmake" "CMake install script"
    safe_remove "$lib_dir/Makefile" "Generated Makefile"
    
    # Find and remove other CMake-generated files
    find "$lib_dir" -name "*.cmake" -not -path "*/cmake/*" -not -name "CMakeLists.txt" 2>/dev/null | while read -r cmake_file; do
        if [[ -f "$cmake_file" ]]; then
            safe_remove "$cmake_file" "CMake generated file"
        fi
    done
}

# Function to clean build artifacts (object files, libraries, executables)
clean_build_artifacts() {
    local lib_dir="$1"
    local lib_name="$2"
    
    print_info "Cleaning build artifacts in $lib_name..."
    
    # Object files
    find "$lib_dir" -name "*.o" -o -name "*.obj" 2>/dev/null | while read -r obj_file; do
        safe_remove "$obj_file" "object file"
    done
    
    # Static libraries
    find "$lib_dir" -name "*.a" -o -name "*.lib" 2>/dev/null | while read -r lib_file; do
        safe_remove "$lib_file" "static library"
    done
    
    # Shared libraries
    find "$lib_dir" -name "*.so" -o -name "*.so.*" -o -name "*.dylib" -o -name "*.dll" 2>/dev/null | while read -r so_file; do
        safe_remove "$so_file" "shared library"
    done
    
    # Build directories that are commonly used
    for build_dir in "build" "Build" "BUILD" ".build"; do
        if [[ -d "$lib_dir/$build_dir" ]]; then
            safe_remove "$lib_dir/$build_dir" "build directory"
        fi
    done
}

# Function to clean executables and final products
clean_executables() {
    local lib_dir="$1"
    local lib_name="$2"
    
    print_info "Cleaning executables in $lib_name..."
    
    # Common executable patterns
    find "$lib_dir" -type f \( -name "*.exe" -o -perm +111 \) 2>/dev/null | while read -r exe_file; do
        # Skip shell scripts and source files
        if [[ "$exe_file" =~ \.(sh|py|pl|rb|js)$ ]]; then
            continue
        fi
        
        # Check if it's likely a compiled executable (not a script)
        if file "$exe_file" 2>/dev/null | grep -q "executable\|Mach-O\|PE32"; then
            safe_remove "$exe_file" "executable"
        fi
    done
    
    # Lua-specific executables
    if [[ "$lib_name" == "lua-5.4.8" ]]; then
        safe_remove "$lib_dir/lua" "Lua interpreter"
        safe_remove "$lib_dir/luac" "Lua compiler"
    fi
}

# Function to clean specific library
clean_library() {
    local lib_name="$1"
    local lib_dir="$THIRDPARTY_DIR/$lib_name"
    
    if [[ ! -d "$lib_dir" ]]; then
        print_warning "Library directory not found: $lib_name"
        return 0
    fi
    
    print_header "Cleaning $lib_name"
    
    # Always clean CMake artifacts and build artifacts
    clean_cmake_artifacts "$lib_dir" "$lib_name"
    clean_build_artifacts "$lib_dir" "$lib_name"
    
    # Clean executables only in full mode
    if [[ "$CLEAN_LEVEL" == "full" ]]; then
        clean_executables "$lib_dir" "$lib_name"
    fi
    
    print_success "Finished cleaning $lib_name"
}

# Function to get list of available libraries
get_available_libraries() {
    if [[ -d "$THIRDPARTY_DIR" ]]; then
        ls -1 "$THIRDPARTY_DIR" 2>/dev/null | sort
    fi
}

# Main execution starts here
print_header "Lua Binding Generator - Thirdparty Cleanup"

# Validate thirdparty directory exists
if [[ ! -d "$THIRDPARTY_DIR" ]]; then
    print_error "Thirdparty directory not found: $THIRDPARTY_DIR"
    exit 1
fi

# Show current configuration
if [[ "$QUIET" != "true" ]]; then
    echo "Configuration:"
    echo "  Clean level: $CLEAN_LEVEL"
    echo "  Target library: ${TARGET_LIBRARY:-all}"
    echo "  Dry run: $DRY_RUN"
    echo "  Backup: $BACKUP"
    echo "  Force: $FORCE"
    echo
fi

# Get list of libraries to clean
if [[ -n "$TARGET_LIBRARY" && "$TARGET_LIBRARY" != "all" ]]; then
    LIBRARIES=("$TARGET_LIBRARY")
else
    # Compatible way to read into array for older bash versions
    LIBRARIES=()
    while IFS= read -r line; do
        LIBRARIES+=("$line")
    done < <(get_available_libraries)
fi

# Show what will be cleaned
if [[ "$QUIET" != "true" ]]; then
    echo "Libraries to clean:"
    for lib in "${LIBRARIES[@]}"; do
        echo "  - $lib"
    done
    echo
fi

# Confirmation prompt (unless forced or dry run)
if [[ "$FORCE" != "true" && "$DRY_RUN" != "true" ]]; then
    echo -n "Continue with cleanup? [y/N]: "
    read -r response
    if [[ ! "$response" =~ ^[Yy]([Ee][Ss])?$ ]]; then
        print_info "Cleanup cancelled by user"
        exit 0
    fi
    echo
fi

# Perform cleanup
start_time=$(date +%s)

for library in "${LIBRARIES[@]}"; do
    clean_library "$library"
done

# Show statistics
end_time=$(date +%s)
elapsed=$((end_time - start_time))

print_header "Cleanup Summary"
echo "Files removed: $DELETED_FILES"
echo "Directories removed: $DELETED_DIRS"
echo "Space freed: $(format_size $FREED_SPACE)"
echo "Time elapsed: ${elapsed}s"

if [[ "$DRY_RUN" == "true" ]]; then
    print_info "This was a dry run. No files were actually deleted."
else
    print_success "Cleanup completed successfully!"
fi

if [[ "$BACKUP" == "true" && "$DRY_RUN" != "true" ]]; then
    print_info "Backups created in: $PROJECT_ROOT/backup_*"
fi