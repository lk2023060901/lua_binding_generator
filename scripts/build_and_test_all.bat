@echo off
setlocal enabledelayedexpansion

REM =============================================================================
REM Lua Binding Generator - Complete Build and Test Script (Windows)
REM =============================================================================
REM 
REM This script performs the complete 4-phase workflow:
REM Phase 1: Build lua_binding_generator tool (examples disabled)
REM Phase 2: Generate C++ to Lua bindings for all examples
REM Phase 3: Reconfigure and compile all example programs (with bindings)
REM Phase 4: Execute all examples for testing
REM
REM Usage: scripts\build_and_test_all.bat [options]
REM Options:
REM   --clean        Clean build directories before building
REM   --verbose      Enable verbose output
REM   --help         Show this help message
REM =============================================================================

REM Script configuration
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%\.."
set "BUILD_DIR=%PROJECT_ROOT%\build"
set "EXAMPLES_DIR=%PROJECT_ROOT%\examples"
set "GENERATED_BINDINGS_DIR=%PROJECT_ROOT%\generated_bindings"

REM Default options
set CLEAN_BUILD=false
set VERBOSE=false
set HELP=false

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :args_done
if "%~1"=="--clean" (
    set CLEAN_BUILD=true
    shift
    goto :parse_args
)
if "%~1"=="--verbose" (
    set VERBOSE=true
    shift
    goto :parse_args
)
if "%~1"=="--help" (
    set HELP=true
    shift
    goto :parse_args
)
echo Unknown option: %~1
echo Use --help for usage information
exit /b 1

:args_done

REM Show help if requested
if "%HELP%"=="true" (
    echo Lua Binding Generator - Complete Build and Test Script
    echo.
    echo Usage: %~nx0 [options]
    echo.
    echo Options:
    echo   --clean        Clean build directories before building
    echo   --verbose      Enable verbose output
    echo   --help         Show this help message
    echo.
    echo This script will:
    echo   Phase 1: Build the lua_binding_generator tool (examples disabled)
    echo   Phase 2: Generate C++ to Lua bindings for all examples
    echo   Phase 3: Reconfigure and compile all example programs (with bindings)
    echo   Phase 4: Execute all examples for testing
    exit /b 0
)

REM Helper functions for colored output
:print_header
echo.
echo === %~1 ===
exit /b 0

:print_success
echo [92m✓ %~1[0m
exit /b 0

:print_warning
echo [93m⚠ %~1[0m
exit /b 0

:print_error
echo [91m✗ %~1[0m
exit /b 0

:print_info
echo [94mℹ %~1[0m
exit /b 0

REM Check prerequisites
:check_prerequisites
call :print_header "Checking Prerequisites"

REM Check if we're in the right directory
if not exist "%PROJECT_ROOT%\CMakeLists.txt" (
    call :print_error "Not in lua_binding_generator project root directory"
    exit /b 1
)

REM Check for CMake
cmake --version >nul 2>&1
if errorlevel 1 (
    call :print_error "CMake is required but not installed"
    exit /b 1
)

REM Check for MSBuild or make
where msbuild >nul 2>&1
set HAS_MSBUILD=!errorlevel!
where make >nul 2>&1
set HAS_MAKE=!errorlevel!

if !HAS_MSBUILD! neq 0 if !HAS_MAKE! neq 0 (
    call :print_error "Either MSBuild or Make is required but neither found"
    exit /b 1
)

call :print_success "All prerequisites satisfied"
exit /b 0

REM Clean build directories
:clean_build
if "%CLEAN_BUILD%"=="true" (
    call :print_header "Cleaning Build Directories"
    
    if exist "%BUILD_DIR%" (
        rmdir /s /q "%BUILD_DIR%"
        call :print_success "Removed build directory"
    )
    
    if exist "%GENERATED_BINDINGS_DIR%" (
        rmdir /s /q "%GENERATED_BINDINGS_DIR%"
        call :print_success "Removed generated bindings directory"
    )
    
    REM Clean any executables in project root
    del "%PROJECT_ROOT%\*_example.exe" >nul 2>&1
    del "%PROJECT_ROOT%\*_test.exe" >nul 2>&1
)
exit /b 0

REM Build lua_binding_generator tool (Phase 1: Tool only)
:build_generator
call :print_header "Building Lua Binding Generator Tool (Phase 1)"

cd /d "%PROJECT_ROOT%"

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

REM Configure with CMake - Phase 1: Only build the tool, no examples
call :print_info "Configuring project with CMake (tool only)..."
if "%VERBOSE%"=="true" (
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF
) else (
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF >nul 2>&1
)

if errorlevel 1 (
    call :print_error "CMake configuration failed"
    exit /b 1
)

REM Build the generator tool
call :print_info "Building lua_binding_generator..."

REM Try different build methods
where msbuild >nul 2>&1
if !errorlevel! equ 0 (
    REM Use MSBuild if available
    if "%VERBOSE%"=="true" (
        msbuild lua_binding_generator.vcxproj /p:Configuration=Release
    ) else (
        msbuild lua_binding_generator.vcxproj /p:Configuration=Release /v:quiet >nul 2>&1
    )
) else (
    REM Fall back to make
    if "%VERBOSE%"=="true" (
        make lua_binding_generator
    ) else (
        make lua_binding_generator >nul 2>&1
    )
)

if errorlevel 1 (
    call :print_error "Build failed"
    exit /b 1
)

REM Check if generator was built successfully
set "GENERATOR_EXE=%BUILD_DIR%\lua_binding_generator.exe"
if not exist "%GENERATOR_EXE%" (
    set "GENERATOR_EXE=%BUILD_DIR%\Release\lua_binding_generator.exe"
)
if not exist "%GENERATOR_EXE%" (
    set "GENERATOR_EXE=%BUILD_DIR%\Debug\lua_binding_generator.exe"
)

if not exist "%GENERATOR_EXE%" (
    call :print_error "Failed to build lua_binding_generator"
    exit /b 1
)

call :print_success "Lua binding generator built successfully"
exit /b 0

REM Generate bindings for all examples
:generate_bindings
call :print_header "Generating Lua Bindings for All Examples (Phase 2)"

cd /d "%PROJECT_ROOT%"

REM Create generated bindings directory
if not exist "%GENERATED_BINDINGS_DIR%" mkdir "%GENERATED_BINDINGS_DIR%"

REM Find all header files in examples directory
set "HEADER_FILES="
for /r "%EXAMPLES_DIR%" %%f in (*.h) do (
    set "HEADER_FILES=!HEADER_FILES! %%f"
)

if "!HEADER_FILES!"=="" (
    call :print_warning "No header files found in examples directory"
    exit /b 0
)

call :print_info "Found header files:"
for /r "%EXAMPLES_DIR%" %%f in (*.h) do (
    echo   - %%~nxf
)

REM Generate bindings
call :print_info "Running lua_binding_generator..."
if "%VERBOSE%"=="true" (
    "%GENERATOR_EXE%" --verbose --output_dir="%GENERATED_BINDINGS_DIR%" !HEADER_FILES!
) else (
    "%GENERATOR_EXE%" --output_dir="%GENERATED_BINDINGS_DIR%" !HEADER_FILES! >nul 2>&1
)

if errorlevel 1 (
    call :print_error "Binding generation failed"
    exit /b 1
)

REM Check generated files
set GENERATED_COUNT=0
if exist "%GENERATED_BINDINGS_DIR%" (
    for %%f in ("%GENERATED_BINDINGS_DIR%\*.cpp") do (
        set /a GENERATED_COUNT+=1
    )
)

if !GENERATED_COUNT! gtr 0 (
    call :print_success "Generated binding files:"
    for %%f in ("%GENERATED_BINDINGS_DIR%\*.cpp") do (
        echo   - %%~nxf
    )
) else (
    call :print_warning "No binding files were generated"
)
exit /b 0

REM Build all example programs
:build_examples
call :print_header "Building All Example Programs (Phase 3)"

cd /d "%BUILD_DIR%"

REM Reconfigure CMake with examples enabled (binding files now exist)
call :print_info "Reconfiguring project with examples enabled..."
if "%VERBOSE%"=="true" (
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON
) else (
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON >nul 2>&1
)

REM Build all example targets
call :print_info "Compiling all examples..."

where msbuild >nul 2>&1
if !errorlevel! equ 0 (
    REM Use MSBuild if available
    if "%VERBOSE%"=="true" (
        msbuild ALL_BUILD.vcxproj /p:Configuration=Release
    ) else (
        msbuild ALL_BUILD.vcxproj /p:Configuration=Release /v:quiet >nul 2>&1
    )
) else (
    REM Fall back to make
    if "%VERBOSE%"=="true" (
        make all
    ) else (
        make all >nul 2>&1
    )
)

REM Check which executables were built
set BUILT_EXAMPLES=
set EXAMPLE_NAMES=simple_example game_engine_example comprehensive_test

for %%name in (%EXAMPLE_NAMES%) do (
    REM Check multiple possible locations
    if exist "%BUILD_DIR%\examples\Release\%%name.exe" (
        set "BUILT_EXAMPLES=!BUILT_EXAMPLES! %BUILD_DIR%\examples\Release\%%name.exe"
    ) else if exist "%BUILD_DIR%\examples\Debug\%%name.exe" (
        set "BUILT_EXAMPLES=!BUILT_EXAMPLES! %BUILD_DIR%\examples\Debug\%%name.exe"
    ) else if exist "%BUILD_DIR%\examples\%%name.exe" (
        set "BUILT_EXAMPLES=!BUILT_EXAMPLES! %BUILD_DIR%\examples\%%name.exe"
    ) else if exist "%BUILD_DIR%\Release\%%name.exe" (
        set "BUILT_EXAMPLES=!BUILT_EXAMPLES! %BUILD_DIR%\Release\%%name.exe"
    ) else if exist "%BUILD_DIR%\Debug\%%name.exe" (
        set "BUILT_EXAMPLES=!BUILT_EXAMPLES! %BUILD_DIR%\Debug\%%name.exe"
    ) else if exist "%BUILD_DIR%\%%name.exe" (
        set "BUILT_EXAMPLES=!BUILT_EXAMPLES! %BUILD_DIR%\%%name.exe"
    )
)

if "!BUILT_EXAMPLES!"=="" (
    call :print_error "No example programs were built successfully"
    exit /b 1
)

call :print_success "Successfully built example programs:"
for %%exe in (!BUILT_EXAMPLES!) do (
    for %%f in ("%%exe") do echo   - %%~nxf
)
exit /b 0

REM Execute all example programs
:run_tests
call :print_header "Running Example Programs (Phase 4)"

set SUCCESS_COUNT=0
set FAILURE_COUNT=0

for %%exe in (!BUILT_EXAMPLES!) do (
    for %%f in ("%%exe") do (
        call :print_info "Running %%~nxf..."
        
        REM Run the executable with timeout
        timeout /t 30 "%%exe" >nul 2>&1
        
        if !errorlevel! equ 0 (
            call :print_success "%%~nxf completed successfully"
            set /a SUCCESS_COUNT+=1
        ) else (
            call :print_error "%%~nxf failed or timed out"
            set /a FAILURE_COUNT+=1
        )
    )
)

call :print_header "Test Results Summary"
call :print_success "Successful executions: !SUCCESS_COUNT!"
if !FAILURE_COUNT! gtr 0 (
    call :print_error "Failed executions: !FAILURE_COUNT!"
) else (
    call :print_success "Failed executions: !FAILURE_COUNT!"
)
exit /b 0

REM Show final summary
:show_summary
call :print_header "Build and Test Summary"

echo Project: Lua Binding Generator
echo Build directory: %BUILD_DIR%
echo Generated bindings: %GENERATED_BINDINGS_DIR%
echo.

REM Check what was generated
set BINDING_COUNT=0
if exist "%GENERATED_BINDINGS_DIR%" (
    for %%f in ("%GENERATED_BINDINGS_DIR%\*.cpp") do (
        set /a BINDING_COUNT+=1
    )
)
echo Generated binding files: !BINDING_COUNT!

REM Check what was built
set EXECUTABLE_COUNT=0
for %%name in (simple_example game_engine_example comprehensive_test) do (
    for %%dir in ("%BUILD_DIR%\examples" "%BUILD_DIR%") do (
        for %%config in ("Release" "Debug" "") do (
            set "CHECK_PATH=%%dir\%%config\%%name.exe"
            if "%%config"=="" set "CHECK_PATH=%%dir\%%name.exe"
            if exist "!CHECK_PATH!" set /a EXECUTABLE_COUNT+=1
        )
    )
)
echo Built executable programs: !EXECUTABLE_COUNT!

echo.
call :print_success "Complete workflow finished successfully!"
echo.
echo You can now:
echo   - Run individual examples from the build directory
echo   - Create Lua test scripts to verify the bindings
echo   - Extend the examples with more C++ features
exit /b 0

REM Main execution
:main
call :print_header "Lua Binding Generator - Complete Build and Test Workflow"
echo Starting automated 4-phase build and test process...
echo Phase 1: Build tool → Phase 2: Generate bindings → Phase 3: Build examples → Phase 4: Test

call :check_prerequisites
if errorlevel 1 exit /b 1

call :clean_build
if errorlevel 1 exit /b 1

call :build_generator
if errorlevel 1 exit /b 1

call :generate_bindings
if errorlevel 1 exit /b 1

call :build_examples
if errorlevel 1 exit /b 1

call :run_tests
if errorlevel 1 exit /b 1

call :show_summary
exit /b 0

REM Run main function
call :main