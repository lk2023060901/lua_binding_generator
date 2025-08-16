@echo off
REM =============================================================================
REM Lua Binding Generator - Third-party Build Artifacts Cleanup Script (Windows)
REM =============================================================================
REM 
REM This script cleans build artifacts from the thirdparty directory to:
REM - Free up disk space (thirdparty builds can be several GB)
REM - Resolve build cache issues
REM - Prepare clean distribution packages
REM - Reset third-party libraries to original state
REM
REM Usage: clean_thirdparty.bat [options]
REM Options:
REM   /level:light|full    Cleaning level (default: light)
REM   /library:name        Clean specific library only
REM   /dryrun              Show what would be deleted without actually deleting
REM   /quiet               Suppress output except errors
REM   /force               Skip confirmation prompts
REM   /backup              Create backup of important build artifacts
REM   /help                Show this help message
REM =============================================================================

setlocal EnableDelayedExpansion

REM Script configuration
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."
set "THIRDPARTY_DIR=%PROJECT_ROOT%\thirdparty"

REM Default options
set "CLEAN_LEVEL=light"
set "TARGET_LIBRARY="
set "DRY_RUN=false"
set "QUIET=false"
set "FORCE=false"
set "BACKUP=false"
set "HELP=false"

REM Counters for statistics
set /a DELETED_FILES=0
set /a DELETED_DIRS=0
set /a FREED_SPACE=0

REM Parse command line arguments
:parse_args
if "%~1"=="" goto end_args
set "arg=%~1"

if /i "%arg:~0,7%"=="/level:" (
    set "CLEAN_LEVEL=%arg:~7%"
    if /i not "!CLEAN_LEVEL!"=="light" if /i not "!CLEAN_LEVEL!"=="full" (
        echo ERROR: Invalid clean level: !CLEAN_LEVEL!. Use 'light' or 'full'
        exit /b 1
    )
) else if /i "%arg:~0,9%"=="/library:" (
    set "TARGET_LIBRARY=%arg:~9%"
) else if /i "%arg%"=="/dryrun" (
    set "DRY_RUN=true"
) else if /i "%arg%"=="/quiet" (
    set "QUIET=true"
) else if /i "%arg%"=="/force" (
    set "FORCE=true"
) else if /i "%arg%"=="/backup" (
    set "BACKUP=true"
) else if /i "%arg%"=="/help" (
    set "HELP=true"
) else (
    echo ERROR: Unknown option: %arg%
    echo Use /help for usage information
    exit /b 1
)

shift
goto parse_args

:end_args

REM Show help if requested
if "%HELP%"=="true" (
    call :show_help
    exit /b 0
)

REM Function to show help
:show_help
echo Lua Binding Generator - Third-party Cleanup Script (Windows)
echo.
echo USAGE:
echo     %~nx0 [OPTIONS]
echo.
echo DESCRIPTION:
echo     Cleans build artifacts from the thirdparty directory. Supports different
echo     cleaning levels and selective library cleaning.
echo.
echo OPTIONS:
echo     /level:light^|full    Cleaning level:
echo                          light: CMake cache, temp files (default)
echo                          full:  All build artifacts, restore to source state
echo.
echo     /library:name        Clean specific library only. Available libraries:
echo                          llvm, clang-tools-extra, lua, sol2, spdlog, zstd, all
echo.
echo     /dryrun              Show what would be deleted without actually deleting
echo     /quiet               Suppress informational output
echo     /force               Skip confirmation prompts
echo     /backup              Create backup of important build artifacts before deletion
echo     /help                Show this help message
echo.
echo EXAMPLES:
echo     %~nx0                         # Light cleanup of all libraries
echo     %~nx0 /level:full             # Complete cleanup of all libraries
echo     %~nx0 /library:lua            # Clean only Lua build artifacts
echo     %~nx0 /dryrun                 # Preview what would be cleaned
echo     %~nx0 /level:full /backup /force  # Full cleanup with backup, no prompts
echo.
echo CLEANING LEVELS:
echo     light: Removes CMake cache, temporary build files, object files
echo     full:  Removes all build artifacts including executables and libraries
goto :eof

REM Function to print colored output (simplified for Windows)
:print_info
if "%QUIET%"=="true" goto :eof
echo [INFO] %~1
goto :eof

:print_success
if "%QUIET%"=="true" goto :eof
echo [SUCCESS] %~1
goto :eof

:print_warning
echo [WARNING] %~1 >&2
goto :eof

:print_error
echo [ERROR] %~1 >&2
goto :eof

:print_header
if "%QUIET%"=="true" goto :eof
echo === %~1 ===
goto :eof

REM Function to get file size (simplified)
:get_file_size
set "file=%~1"
set "size=0"
if exist "%file%" (
    for %%A in ("%file%") do set "size=%%~zA"
)
set "%~2=%size%"
goto :eof

REM Function to format file size
:format_size
set /a size=%1
if %size% LSS 1024 (
    set "formatted=%size%B"
) else if %size% LSS 1048576 (
    set /a kb=size/1024
    set "formatted=!kb!KB"
) else if %size% LSS 1073741824 (
    set /a mb=size/1048576
    set "formatted=!mb!MB"
) else (
    set /a gb=size/1073741824
    set "formatted=!gb!GB"
)
set "%~2=%formatted%"
goto :eof

REM Function to safely remove file or directory
:safe_remove
set "target=%~1"
set "description=%~2"

if not exist "%target%" goto :eof

set "size=0"
if exist "%target%\" (
    REM It's a directory
    set /a DELETED_DIRS+=1
    REM Get directory size (approximation)
    for /f "tokens=3" %%A in ('dir "%target%" /s /-c /q 2^>nul ^| findstr /r /c:"[0-9][0-9]* File(s)"') do set "size=%%A"
) else (
    REM It's a file
    set /a DELETED_FILES+=1
    call :get_file_size "%target%" size
)

set /a FREED_SPACE+=size

if "%DRY_RUN%"=="true" (
    call :format_size %size% formatted
    call :print_info "[DRY RUN] Would remove %description%: %target% (!formatted!)"
    goto :eof
)

if "%BACKUP%"=="true" if exist "%target%" (
    set "backup_dir=%PROJECT_ROOT%\backup_%date:~-4%%date:~4,2%%date:~7,2%_%time:~0,2%%time:~3,2%%time:~6,2%"
    set "backup_dir=!backup_dir: =0!"
    set "backup_target=!backup_dir!\thirdparty\%~nx1"
    
    call :print_info "Creating backup: !backup_target!"
    if not exist "!backup_dir!\thirdparty" mkdir "!backup_dir!\thirdparty" 2>nul
    if exist "%target%\" (
        xcopy "%target%" "!backup_target!\" /E /I /Q >nul 2>&1
    ) else (
        copy "%target%" "!backup_target!" >nul 2>&1
    )
)

call :format_size %size% formatted
call :print_info "Removing %description%: %~nx1 (!formatted!)"

if exist "%target%\" (
    rmdir /s /q "%target%" 2>nul
) else (
    del /f /q "%target%" 2>nul
)
goto :eof

REM Function to clean CMake artifacts
:clean_cmake_artifacts
set "lib_dir=%~1"
set "lib_name=%~2"

call :print_info "Cleaning CMake artifacts in %lib_name%..."

REM CMake cache and configuration files
call :safe_remove "%lib_dir%\CMakeCache.txt" "CMake cache"
call :safe_remove "%lib_dir%\CMakeFiles" "CMake files directory"
call :safe_remove "%lib_dir%\cmake_install.cmake" "CMake install script"
call :safe_remove "%lib_dir%\Makefile" "Generated Makefile"

REM Find and remove other CMake-generated files
for /r "%lib_dir%" %%F in (*.cmake) do (
    set "cmake_file=%%F"
    set "file_name=%%~nxF"
    if /i not "!file_name!"=="CMakeLists.txt" (
        if not "!cmake_file:cmake=!" == "!cmake_file!" (
            REM Skip files in cmake directories
        ) else (
            call :safe_remove "!cmake_file!" "CMake generated file"
        )
    )
)
goto :eof

REM Function to clean build artifacts
:clean_build_artifacts
set "lib_dir=%~1"
set "lib_name=%~2"

call :print_info "Cleaning build artifacts in %lib_name%..."

REM Object files
for /r "%lib_dir%" %%F in (*.o *.obj) do (
    call :safe_remove "%%F" "object file"
)

REM Static libraries
for /r "%lib_dir%" %%F in (*.a *.lib) do (
    call :safe_remove "%%F" "static library"
)

REM Shared libraries
for /r "%lib_dir%" %%F in (*.so *.dll *.dylib) do (
    call :safe_remove "%%F" "shared library"
)

REM Build directories
for %%D in (build Build BUILD .build) do (
    if exist "%lib_dir%\%%D" (
        call :safe_remove "%lib_dir%\%%D" "build directory"
    )
)
goto :eof

REM Function to clean executables
:clean_executables
set "lib_dir=%~1"
set "lib_name=%~2"

call :print_info "Cleaning executables in %lib_name%..."

REM Executable files
for /r "%lib_dir%" %%F in (*.exe) do (
    call :safe_remove "%%F" "executable"
)

REM Lua-specific executables (without .exe extension on Windows they might not exist)
if /i "%lib_name%"=="lua-5.4.8" (
    call :safe_remove "%lib_dir%\lua.exe" "Lua interpreter"
    call :safe_remove "%lib_dir%\luac.exe" "Lua compiler"
    call :safe_remove "%lib_dir%\lua" "Lua interpreter"
    call :safe_remove "%lib_dir%\luac" "Lua compiler"
)
goto :eof

REM Function to clean specific library
:clean_library
set "lib_name=%~1"
set "lib_dir=%THIRDPARTY_DIR%\%lib_name%"

if not exist "%lib_dir%" (
    call :print_warning "Library directory not found: %lib_name%"
    goto :eof
)

call :print_header "Cleaning %lib_name%"

REM Always clean CMake artifacts and build artifacts
call :clean_cmake_artifacts "%lib_dir%" "%lib_name%"
call :clean_build_artifacts "%lib_dir%" "%lib_name%"

REM Clean executables only in full mode
if /i "%CLEAN_LEVEL%"=="full" (
    call :clean_executables "%lib_dir%" "%lib_name%"
)

call :print_success "Finished cleaning %lib_name%"
goto :eof

REM Main execution starts here
call :print_header "Lua Binding Generator - Thirdparty Cleanup"

REM Validate thirdparty directory exists
if not exist "%THIRDPARTY_DIR%" (
    call :print_error "Thirdparty directory not found: %THIRDPARTY_DIR%"
    exit /b 1
)

REM Show current configuration
if "%QUIET%"=="false" (
    echo Configuration:
    echo   Clean level: %CLEAN_LEVEL%
    if "%TARGET_LIBRARY%"=="" (
        echo   Target library: all
    ) else (
        echo   Target library: %TARGET_LIBRARY%
    )
    echo   Dry run: %DRY_RUN%
    echo   Backup: %BACKUP%
    echo   Force: %FORCE%
    echo.
)

REM Get list of libraries to clean
if "%TARGET_LIBRARY%"=="" set "TARGET_LIBRARY=all"
if /i "%TARGET_LIBRARY%"=="all" (
    REM Get all subdirectories
    set "LIBRARIES="
    for /d %%D in ("%THIRDPARTY_DIR%\*") do (
        set "LIBRARIES=!LIBRARIES! %%~nxD"
    )
) else (
    set "LIBRARIES=%TARGET_LIBRARY%"
)

REM Show what will be cleaned
if "%QUIET%"=="false" (
    echo Libraries to clean:
    for %%L in (%LIBRARIES%) do (
        echo   - %%L
    )
    echo.
)

REM Confirmation prompt (unless forced or dry run)
if "%FORCE%"=="false" if "%DRY_RUN%"=="false" (
    set /p "response=Continue with cleanup? [y/N]: "
    if /i not "!response!"=="y" if /i not "!response!"=="yes" (
        call :print_info "Cleanup cancelled by user"
        exit /b 0
    )
    echo.
)

REM Perform cleanup
set "start_time=%time%"

for %%L in (%LIBRARIES%) do (
    call :clean_library "%%L"
)

REM Show statistics
set "end_time=%time%"
call :print_header "Cleanup Summary"
echo Files removed: %DELETED_FILES%
echo Directories removed: %DELETED_DIRS%
call :format_size %FREED_SPACE% formatted_space
echo Space freed: %formatted_space%

if "%DRY_RUN%"=="true" (
    call :print_info "This was a dry run. No files were actually deleted."
) else (
    call :print_success "Cleanup completed successfully!"
)

if "%BACKUP%"=="true" if "%DRY_RUN%"=="false" (
    call :print_info "Backups created in: %PROJECT_ROOT%\backup_*"
)

endlocal