--[[
run_all_tests.lua
运行所有Lua绑定测试的主脚本

这个脚本会依次运行所有的测试文件并汇总结果
]]

print("=== Lua Binding Generator - Test Suite ===")
print("Running comprehensive test suite for all examples...")
print()

-- 测试结果统计
local total_suites = 0
local passed_suites = 0
local failed_suites = 0

-- 颜色输出支持（如果终端支持）
local function colored_print(text, color)
    -- 简单的ANSI颜色代码
    local colors = {
        red = "\27[31m",
        green = "\27[32m",
        yellow = "\27[33m",
        blue = "\27[34m",
        reset = "\27[0m"
    }
    
    local color_code = colors[color] or ""
    local reset_code = colors.reset or ""
    print(color_code .. text .. reset_code)
end

-- 运行单个测试文件的函数
local function run_test_file(test_file, description)
    total_suites = total_suites + 1
    
    print("📋 Running: " .. description)
    print("   File: " .. test_file)
    print("   " .. string.rep("-", 50))
    
    local start_time = os.clock()
    
    -- 尝试加载并运行测试文件
    local success, result = pcall(function()
        return dofile(test_file)
    end)
    
    local end_time = os.clock()
    local duration = end_time - start_time
    
    if success then
        if result == 0 or result == nil then
            colored_print(string.format("✅ PASSED: %s (%.3fs)", description, duration), "green")
            passed_suites = passed_suites + 1
        else
            colored_print(string.format("❌ FAILED: %s - Exit code: %s (%.3fs)", description, tostring(result), duration), "red")
            failed_suites = failed_suites + 1
        end
    else
        colored_print(string.format("💥 ERROR: %s - %s (%.3fs)", description, tostring(result), duration), "red")
        failed_suites = failed_suites + 1
    end
    
    print()
end

-- 获取脚本所在目录
local script_dir = debug.getinfo(1, "S").source:match("@(.*/)")
if not script_dir then
    script_dir = "./"
end

print("Script directory: " .. script_dir)
print()

-- 定义所有测试文件
local test_files = {
    {
        file = script_dir .. "test_simple.lua",
        description = "Simple Example Bindings",
        required = true
    },
    {
        file = script_dir .. "test_game_engine.lua", 
        description = "Game Engine Bindings",
        required = true
    },
    {
        file = script_dir .. "test_comprehensive.lua",
        description = "Comprehensive Feature Test",
        required = true
    }
}

-- 运行所有测试
print("🚀 Starting test execution...")
print()

for i, test_info in ipairs(test_files) do
    local file_path = test_info.file
    local description = test_info.description
    local required = test_info.required
    
    -- 检查文件是否存在
    local file = io.open(file_path, "r")
    if file then
        file:close()
        run_test_file(file_path, description)
    else
        if required then
            total_suites = total_suites + 1
            failed_suites = failed_suites + 1
            colored_print("📁 MISSING: " .. description .. " - File not found: " .. file_path, "yellow")
            print()
        else
            colored_print("⏭️  SKIPPED: " .. description .. " - Optional test file not found", "yellow")
            print()
        end
    end
end

-- 生成测试报告
print("📊 " .. string.rep("=", 60))
print("📊 TEST SUITE SUMMARY")
print("📊 " .. string.rep("=", 60))
print()

local success_rate = 0
if total_suites > 0 then
    success_rate = (passed_suites / total_suites) * 100
end

print(string.format("Total test suites: %d", total_suites))

if passed_suites > 0 then
    colored_print(string.format("✅ Passed: %d", passed_suites), "green")
end

if failed_suites > 0 then
    colored_print(string.format("❌ Failed: %d", failed_suites), "red")
end

print(string.format("Success rate: %.1f%%", success_rate))
print()

-- 最终结果
if failed_suites == 0 and total_suites > 0 then
    colored_print("🎉 ALL TESTS PASSED! 🎉", "green")
    colored_print("The Lua binding generator is working correctly!", "green")
    print()
    print("✨ Next steps:")
    print("   1. Generate bindings: lua_binding_generator examples/*.h")
    print("   2. Compile with Sol2 integration")
    print("   3. Run the generated Lua bindings in your application")
    return 0
elseif total_suites == 0 then
    colored_print("⚠️  NO TESTS FOUND", "yellow")
    colored_print("Please ensure test files are present in the scripts directory.", "yellow")
    print()
    print("Expected test files:")
    for _, test_info in ipairs(test_files) do
        print("  - " .. test_info.file)
    end
    return 1
else
    colored_print("💔 SOME TESTS FAILED", "red")
    colored_print(string.format("%d out of %d test suites failed.", failed_suites, total_suites), "red")
    print()
    print("🔧 Troubleshooting steps:")
    print("   1. Check that the lua_binding_generator has generated bindings")
    print("   2. Verify that all C++ classes are properly exported")
    print("   3. Ensure Sol2 is correctly integrated")
    print("   4. Check for any compilation errors in the binding code")
    return 1
end