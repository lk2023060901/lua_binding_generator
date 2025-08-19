--[[
    @file run_all_tests.lua
    @brief 运行所有Lua测试脚本的主脚本
    
    这个脚本会依次执行所有的测试脚本，
    并生成综合的测试报告。
]]

print("================================================================")
print("           Lua Binding Generator - Lua端测试套件")
print("                        v2.0.0")
print("================================================================")

-- 测试脚本列表
local test_scripts = {
    {
        name = "基础宏功能测试",
        file = "basic_macro_test.lua",
        description = "测试常量、变量、枚举、函数导出"
    },
    {
        name = "类绑定测试",
        file = "class_binding_test.lua",
        description = "测试普通类、静态类、单例类、管理器类"
    },
    {
        name = "运算符重载测试", 
        file = "operator_overload_test.lua",
        description = "测试向量类的各种运算符重载"
    },
    {
        name = "回调和容器测试",
        file = "callback_container_test.lua", 
        description = "测试事件系统、STL容器绑定"
    },
    {
        name = "运行时集成测试",
        file = "runtime_integration_test.lua",
        description = "测试运行时库功能和集成"
    }
}

-- 测试结果统计
local total_tests = 0
local passed_tests = 0
local failed_tests = 0
local test_results = {}
local start_time = os.clock()

-- 工具函数：安全执行Lua文件
local function safe_dofile(filename)
    local success, result = pcall(dofile, filename)
    return success, result
end

-- 工具函数：格式化时间
local function format_time(seconds)
    if seconds < 1 then
        return string.format("%.0f ms", seconds * 1000)
    else
        return string.format("%.2f s", seconds)
    end
end

-- 工具函数：打印测试结果
local function print_test_result(name, success, duration, error_msg)
    local status = success and "✅ PASS" or "❌ FAIL"
    local time_str = format_time(duration)
    
    print(string.format("[%s] %s (%s)", status, name, time_str))
    
    if not success and error_msg then
        print("   错误: " .. tostring(error_msg))
    end
end

-- 运行单个测试脚本
local function run_test_script(test_info)
    print("\n" .. string.rep("-", 60))
    print("运行测试: " .. test_info.name)
    print("描述: " .. test_info.description)
    print("文件: " .. test_info.file)
    print(string.rep("-", 60))
    
    local test_start = os.clock()
    local success, result = safe_dofile(test_info.file)
    local test_duration = os.clock() - test_start
    
    total_tests = total_tests + 1
    
    local test_result = {
        name = test_info.name,
        file = test_info.file,
        success = success,
        duration = test_duration,
        result = result,
        error_msg = not success and result or nil
    }
    
    table.insert(test_results, test_result)
    
    if success and result == true then
        passed_tests = passed_tests + 1
        print_test_result(test_info.name, true, test_duration)
    else
        failed_tests = failed_tests + 1
        local error_msg = not success and result or "测试返回false或nil"
        print_test_result(test_info.name, false, test_duration, error_msg)
    end
end

-- 主测试循环
print("开始执行Lua端测试...")
print("测试脚本数量: " .. #test_scripts)

for i, test_info in ipairs(test_scripts) do
    print(string.format("\n[%d/%d] 准备运行: %s", i, #test_scripts, test_info.name))
    
    -- 检查文件是否存在
    local file_handle = io.open(test_info.file, "r")
    if file_handle then
        file_handle:close()
        run_test_script(test_info)
    else
        print("❌ 文件不存在: " .. test_info.file)
        total_tests = total_tests + 1
        failed_tests = failed_tests + 1
        
        table.insert(test_results, {
            name = test_info.name,
            file = test_info.file,
            success = false,
            duration = 0,
            error_msg = "文件不存在"
        })
    end
end

-- 计算总时间
local total_duration = os.clock() - start_time

-- 生成测试报告
print("\n" .. string.rep("=", 60))
print("                    测试结果摘要")
print(string.rep("=", 60))

print(string.format("总测试数:     %d", total_tests))
print(string.format("通过测试:     %d", passed_tests))
print(string.format("失败测试:     %d", failed_tests))

if total_tests > 0 then
    local success_rate = (passed_tests / total_tests) * 100
    print(string.format("成功率:       %.1f%%", success_rate))
end

print(string.format("总执行时间:   %s", format_time(total_duration)))

-- 详细结果
print("\n" .. string.rep("-", 60))
print("详细测试结果:")
print(string.rep("-", 60))

for i, result in ipairs(test_results) do
    local status_icon = result.success and "✅" or "❌"
    local status_text = result.success and "PASS" or "FAIL"
    local time_str = format_time(result.duration)
    
    print(string.format("%d. %s [%s] %s (%s)", 
          i, status_icon, status_text, result.name, time_str))
    
    if not result.success then
        print("   📁 文件: " .. result.file)
        if result.error_msg then
            print("   ⚠️  错误: " .. tostring(result.error_msg))
        end
    end
end

-- 失败测试详情
if failed_tests > 0 then
    print("\n" .. string.rep("-", 60))
    print("失败测试详情:")
    print(string.rep("-", 60))
    
    for i, result in ipairs(test_results) do
        if not result.success then
            print(string.format("\n%d. %s", i, result.name))
            print("   文件: " .. result.file)
            if result.error_msg then
                print("   错误信息:")
                -- 分行显示错误信息
                for line in tostring(result.error_msg):gmatch("[^\n]+") do
                    print("     " .. line)
                end
            end
        end
    end
end

-- 测试分类统计
print("\n" .. string.rep("-", 60))
print("测试分类统计:")
print(string.rep("-", 60))

local categories = {
    ["基础功能"] = {"基础宏功能测试"},
    ["类系统"] = {"类绑定测试"},
    ["运算符"] = {"运算符重载测试"},
    ["高级功能"] = {"回调和容器测试"},
    ["运行时库"] = {"运行时集成测试"}
}

for category, test_names in pairs(categories) do
    local category_passed = 0
    local category_total = 0
    
    for _, test_name in ipairs(test_names) do
        for _, result in ipairs(test_results) do
            if result.name == test_name then
                category_total = category_total + 1
                if result.success then
                    category_passed = category_passed + 1
                end
                break
            end
        end
    end
    
    local category_rate = category_total > 0 and (category_passed / category_total) * 100 or 0
    local status_icon = category_rate == 100 and "✅" or category_rate > 0 and "⚠️" or "❌"
    
    print(string.format("%s %s: %d/%d (%.1f%%)", 
          status_icon, category, category_passed, category_total, category_rate))
end

-- 建议和下一步
print("\n" .. string.rep("-", 60))
print("建议和下一步:")
print(string.rep("-", 60))

if failed_tests == 0 then
    print("🎉 恭喜！所有Lua端测试都通过了！")
    print("")
    print("✅ Lua绑定生成正确")
    print("✅ 所有导出功能可用")
    print("✅ 运行时库集成正常")
    print("")
    print("下一步可以:")
    print("1. 运行C++端的完整测试套件")
    print("2. 进行性能基准测试")
    print("3. 测试实际应用场景")
    
elseif failed_tests < total_tests / 2 then
    print("⚠️  大部分测试通过，但仍有一些问题需要解决。")
    print("")
    print("建议:")
    print("1. 检查失败的测试，确认是否是绑定生成问题")
    print("2. 验证运行时库是否正确链接")
    print("3. 确认所有依赖库都已安装")
    
else
    print("❌ 多个测试失败，可能存在严重问题。")
    print("")
    print("建议:")
    print("1. 检查lua_binding_generator是否正确运行")
    print("2. 确认生成的绑定文件存在且正确")
    print("3. 验证编译配置和依赖库")
    print("4. 查看详细错误信息进行调试")
end

-- 保存测试报告到文件
local function save_test_report()
    local report_file = "lua_test_report.txt"
    local file = io.open(report_file, "w")
    
    if file then
        file:write("Lua Binding Generator - Lua端测试报告\n")
        file:write("生成时间: " .. os.date("%Y-%m-%d %H:%M:%S") .. "\n")
        file:write(string.rep("=", 50) .. "\n\n")
        
        file:write("测试摘要:\n")
        file:write(string.format("总测试数: %d\n", total_tests))
        file:write(string.format("通过测试: %d\n", passed_tests))
        file:write(string.format("失败测试: %d\n", failed_tests))
        
        if total_tests > 0 then
            local success_rate = (passed_tests / total_tests) * 100
            file:write(string.format("成功率: %.1f%%\n", success_rate))
        end
        
        file:write(string.format("总执行时间: %s\n\n", format_time(total_duration)))
        
        file:write("详细结果:\n")
        for i, result in ipairs(test_results) do
            local status = result.success and "PASS" or "FAIL"
            file:write(string.format("%d. [%s] %s (%s)\n", 
                      i, status, result.name, format_time(result.duration)))
            
            if not result.success and result.error_msg then
                file:write("   错误: " .. tostring(result.error_msg) .. "\n")
            end
        end
        
        file:close()
        print("\n📄 测试报告已保存到: " .. report_file)
    else
        print("\n⚠️  无法保存测试报告文件")
    end
end

save_test_report()

-- 最终状态输出
print("\n" .. string.rep("=", 60))
if failed_tests == 0 then
    print("🎉 所有Lua端测试完成 - 全部通过！")
    print("lua_binding_generator的Lua绑定功能正常。")
else
    print(string.format("⚠️  Lua端测试完成 - %d个测试失败", failed_tests))
    print("请检查上述错误信息并进行相应的修复。")
end
print(string.rep("=", 60))

-- 返回整体测试结果
return failed_tests == 0