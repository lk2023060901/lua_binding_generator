--[[
run_tests_safe.lua
安全版本的测试运行器

这个脚本可以在没有实际绑定的情况下运行，会检查模块是否可用
]]

print("=== Lua Binding Generator - Safe Test Suite ===")
print("Testing framework and checking for available bindings...")
print()

-- 测试结果统计
local total_suites = 0
local passed_suites = 0
local failed_suites = 0
local skipped_suites = 0

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

-- 检查模块是否可用
local function check_module(module_name)
    local success, module = pcall(require, module_name)
    if success and module then
        return true
    end
    
    -- 尝试作为全局变量访问
    if _G[module_name] then
        return true
    end
    
    return false
end

-- 运行单个测试文件的函数
local function run_test_file(test_file, description, required_modules)
    total_suites = total_suites + 1
    
    print("📋 Testing: " .. description)
    print("   File: " .. test_file)
    
    -- 检查是否存在测试文件
    local file = io.open(test_file, "r")
    if not file then
        colored_print("📁 MISSING: Test file not found - " .. test_file, "yellow")
        skipped_suites = skipped_suites + 1
        print()
        return
    end
    file:close()
    
    -- 检查必需的模块
    if required_modules then
        local missing_modules = {}
        for _, module_name in ipairs(required_modules) do
            if not check_module(module_name) then
                table.insert(missing_modules, module_name)
            end
        end
        
        if #missing_modules > 0 then
            colored_print("⏭️  SKIPPED: Missing required modules: " .. table.concat(missing_modules, ", "), "yellow")
            colored_print("   💡 Generate bindings first: lua_binding_generator --output-dir=generated_bindings examples/*.h", "blue")
            skipped_suites = skipped_suites + 1
            print()
            return
        end
    end
    
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

-- 定义所有测试文件和它们的依赖
local test_files = {
    {
        file = script_dir .. "test_framework_standalone.lua",
        description = "Test Framework Validation",
        required_modules = nil  -- 不需要任何外部模块
    },
    {
        file = script_dir .. "test_simple.lua",
        description = "Simple Example Bindings",
        required_modules = {"simple"}
    },
    {
        file = script_dir .. "test_game_engine.lua", 
        description = "Game Engine Bindings",
        required_modules = {"engine"}
    },
    {
        file = script_dir .. "test_comprehensive.lua",
        description = "Comprehensive Feature Test",
        required_modules = {"game"}
    },
    {
        file = script_dir .. "test_bindings_integration.lua",
        description = "Integration Test Suite",
        required_modules = {"simple", "engine", "game"}
    }
}

-- 运行所有测试
print("🚀 Starting safe test execution...")
print()

for i, test_info in ipairs(test_files) do
    run_test_file(test_info.file, test_info.description, test_info.required_modules)
end

-- 生成测试报告
print("📊 " .. string.rep("=", 60))
print("📊 SAFE TEST SUITE SUMMARY")
print("📊 " .. string.rep("=", 60))
print()

local total_attempted = total_suites
local success_rate = 0
if (passed_suites + failed_suites) > 0 then
    success_rate = (passed_suites / (passed_suites + failed_suites)) * 100
end

print(string.format("Total test suites: %d", total_suites))

if passed_suites > 0 then
    colored_print(string.format("✅ Passed: %d", passed_suites), "green")
end

if failed_suites > 0 then
    colored_print(string.format("❌ Failed: %d", failed_suites), "red")
end

if skipped_suites > 0 then
    colored_print(string.format("⏭️  Skipped: %d (missing modules)", skipped_suites), "yellow")
end

if (passed_suites + failed_suites) > 0 then
    print(string.format("Success rate: %.1f%% (of executed tests)", success_rate))
end
print()

-- 最终结果和建议
if failed_suites == 0 and passed_suites > 0 then
    colored_print("🎉 ALL EXECUTED TESTS PASSED! 🎉", "green")
    if skipped_suites > 0 then
        print()
        colored_print("📋 Next steps to run skipped tests:", "blue")
        print("   1. Generate bindings: lua_binding_generator --output-dir=generated_bindings examples/*.h")
        print("   2. Integrate bindings into Sol2-enabled C++ application")
        print("   3. Run tests again with integrated bindings")
    else
        colored_print("All binding tests are working correctly!", "green")
    end
    return 0
elseif total_suites == skipped_suites then
    colored_print("⚠️  ALL TESTS SKIPPED - NO BINDINGS AVAILABLE", "yellow")
    print()
    colored_print("🔧 To run binding tests:", "blue")
    print("   1. First, make sure lua_binding_generator is built:")
    print("      cd build && make")
    print("   2. Generate bindings:")
    print("      ./build/lua_binding_generator --output-dir=generated_bindings examples/*.h")
    print("   3. Integrate generated bindings into a Sol2-enabled application")
    print("   4. Run tests from within the integrated application")
    return 1
else
    colored_print("💔 SOME TESTS FAILED OR HAD ERRORS", "red")
    if skipped_suites > 0 then
        colored_print(string.format("Additional %d tests were skipped due to missing bindings", skipped_suites), "yellow")
    end
    print()
    colored_print("🔧 Troubleshooting:", "blue")
    print("   1. Check test error messages above")
    print("   2. Ensure all required modules are properly loaded")
    print("   3. Verify binding generation and integration")
    return 1
end