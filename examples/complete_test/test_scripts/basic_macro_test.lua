--[[
    @file basic_macro_test.lua
    @brief 基础宏功能测试脚本
    
    这个Lua脚本用于测试从C++绑定导出的基础功能，
    验证15个核心宏的导出是否正常工作。
]]

print("=== 基础宏功能测试 ===")

-- 测试常量导出
print("\n--- 常量测试 ---")
if MAX_CONNECTIONS then
    print("✅ MAX_CONNECTIONS =", MAX_CONNECTIONS)
    assert(MAX_CONNECTIONS == 1000, "MAX_CONNECTIONS should be 1000")
else
    error("❌ MAX_CONNECTIONS not found")
end

if PI_VALUE then
    print("✅ PI_VALUE =", PI_VALUE)
    assert(math.abs(PI_VALUE - 3.14159) < 0.01, "PI_VALUE should be approximately 3.14159")
else
    error("❌ PI_VALUE not found")
end

if TEST_VERSION then
    print("✅ TEST_VERSION =", TEST_VERSION)
    assert(TEST_VERSION == "2.0.0", "TEST_VERSION should be '2.0.0'")
else
    error("❌ TEST_VERSION not found")
end

-- 测试变量导出
print("\n--- 变量测试 ---")
if global_counter ~= nil then
    print("✅ global_counter =", global_counter)
    global_counter = 100
    print("   设置 global_counter = 100")
else
    error("❌ global_counter not found")
end

if system_name then
    print("✅ system_name =", system_name)
    assert(system_name == "MacroCoverageTest", "system_name should be 'MacroCoverageTest'")
else
    error("❌ system_name not found")
end

-- 测试枚举导出
print("\n--- 枚举测试 ---")
if TestStatus then
    print("✅ TestStatus enum found")
    print("   INACTIVE =", TestStatus.INACTIVE)
    print("   ACTIVE =", TestStatus.ACTIVE)
    print("   PENDING =", TestStatus.PENDING)
    print("   ERROR =", TestStatus.ERROR)
    
    assert(TestStatus.INACTIVE == 0, "INACTIVE should be 0")
    assert(TestStatus.ACTIVE == 1, "ACTIVE should be 1")
    assert(TestStatus.ERROR == -1, "ERROR should be -1")
else
    error("❌ TestStatus enum not found")
end

if TestPriority then
    print("✅ TestPriority enum found")
    print("   LOW =", TestPriority.LOW)
    print("   MEDIUM =", TestPriority.MEDIUM)
    print("   HIGH =", TestPriority.HIGH)
    print("   CRITICAL =", TestPriority.CRITICAL)
    
    assert(TestPriority.HIGH == 10, "HIGH should be 10")
    assert(TestPriority.CRITICAL == 100, "CRITICAL should be 100")
else
    error("❌ TestPriority enum not found")
end

-- 测试函数导出
print("\n--- 函数测试 ---")
if add_numbers then
    local result = add_numbers(15, 25)
    print("✅ add_numbers(15, 25) =", result)
    assert(result == 40, "add_numbers(15, 25) should be 40")
else
    error("❌ add_numbers function not found")
end

if format_message then
    local message = format_message("测试消息: {}", "成功")
    print("✅ format_message =", message)
    assert(message:find("成功"), "Formatted message should contain '成功'")
else
    error("❌ format_message function not found")
end

if generate_sequence then
    local seq = generate_sequence(1, 10, 2)
    print("✅ generate_sequence(1, 10, 2) =", table.concat(seq, ", "))
    assert(#seq == 5, "Sequence should have 5 elements")
    assert(seq[1] == 1 and seq[2] == 3, "Sequence should start with 1, 3")
else
    error("❌ generate_sequence function not found")
end

-- 测试重载函数
if calculate_area then
    local circle_area = calculate_area(5.0)  -- 单参数版本（圆形）
    local rect_area = calculate_area(4.0, 3.0)  -- 双参数版本（矩形）
    
    print("✅ calculate_area(5.0) =", circle_area)
    print("✅ calculate_area(4.0, 3.0) =", rect_area)
    
    assert(circle_area > 78 and circle_area < 79, "Circle area should be approximately 78.5")
    assert(rect_area == 12, "Rectangle area should be 12")
else
    error("❌ calculate_area function not found")
end

print("\n🎉 基础宏功能测试全部通过！")
return true