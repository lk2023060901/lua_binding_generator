--[[
test_simple.lua
Simple Example Lua绑定测试脚本

测试simple_example.h中定义的所有导出项
]]

-- 测试结果统计
local tests_passed = 0
local tests_failed = 0

-- 测试辅助函数
function assert_equal(actual, expected, message)
    if actual == expected then
        tests_passed = tests_passed + 1
        print("✓ PASS: " .. (message or "test"))
    else
        tests_failed = tests_failed + 1
        print("✗ FAIL: " .. (message or "test") .. " - Expected: " .. tostring(expected) .. ", Got: " .. tostring(actual))
    end
end

function assert_true(condition, message)
    assert_equal(condition, true, message)
end

function assert_false(condition, message)
    assert_equal(condition, false, message)
end

print("=== Simple Example Lua Binding Tests ===")

-- 检查simple模块是否可用
if not simple then
    print("❌ ERROR: 'simple' module is not available")
    print("   Make sure to:")
    print("   1. Generate bindings: lua_binding_generator examples/simple_example.h")
    print("   2. Integrate bindings into Sol2-enabled application")
    print("   3. Load the simple module in Lua")
    print("")
    print("=== Test Summary ===")
    print("Tests passed: 0")
    print("Tests failed: 0") 
    print("Total tests: 0")
    print("⚠️ Module not available - tests skipped")
    return 1
end

-- 测试枚举
print("\n--- Testing Enums ---")
assert_equal(simple.Color.RED, 0, "Color.RED value")
assert_equal(simple.Color.GREEN, 1, "Color.GREEN value")
assert_equal(simple.Color.BLUE, 2, "Color.BLUE value")
assert_equal(simple.Color.YELLOW, 3, "Color.YELLOW value")

-- 测试常量
print("\n--- Testing Constants ---")
assert_equal(simple.MAX_COUNT, 10, "MAX_COUNT constant")
assert_equal(simple.VERSION, 1.0, "VERSION constant")
assert_equal(simple.APP_NAME, "Simple Example", "APP_NAME constant")

-- 测试全局函数
print("\n--- Testing Global Functions ---")
assert_equal(simple.add(5, 3), 8, "add(5, 3)")
assert_equal(simple.add(-2, 7), 5, "add(-2, 7)")

assert_equal(simple.greet("World"), "Hello, World!", "greet('World')")
assert_equal(simple.greet("Lua"), "Hello, Lua!", "greet('Lua')")

assert_true(simple.isEven(42), "isEven(42)")
assert_false(simple.isEven(43), "isEven(43)")
assert_true(simple.isEven(0), "isEven(0)")
assert_false(simple.isEven(-1), "isEven(-1)")

-- 测试Calculator类
print("\n--- Testing Calculator Class ---")

-- 创建Calculator实例
local calc = simple.Calculator.new()
assert_equal(calc:getValue(), 0.0, "Calculator default constructor")

local calc2 = simple.Calculator.new(10.0)
assert_equal(calc2:getValue(), 10.0, "Calculator parameterized constructor")

-- 测试属性访问
calc:setValue(5.0)
assert_equal(calc:getValue(), 5.0, "Calculator setValue/getValue")

-- 测试方法
assert_equal(calc:add(3.0), 8.0, "Calculator add method")
assert_equal(calc:getValue(), 8.0, "Calculator value after add")

assert_equal(calc:multiply(2.0), 16.0, "Calculator multiply method")
assert_equal(calc:subtract(6.0), 10.0, "Calculator subtract method")
assert_equal(calc:divide(2.0), 5.0, "Calculator divide method")

-- 测试除零保护
calc:setValue(10.0)
calc:divide(0.0)  -- 应该不改变值
assert_equal(calc:getValue(), 10.0, "Calculator divide by zero protection")

-- 测试reset和clear
calc:reset()
assert_equal(calc:getValue(), 0.0, "Calculator reset")

calc:setValue(100.0)
calc:clear()
assert_equal(calc:getValue(), 0.0, "Calculator clear")

-- 测试静态方法
local pi_value = simple.Calculator.pi()
assert_true(math.abs(pi_value - 3.141592653589793) < 1e-10, "Calculator.pi() static method")

local e_value = simple.Calculator.e()
assert_true(math.abs(e_value - 2.718281828459045) < 1e-10, "Calculator.e() static method")

-- 测试DataContainer类
print("\n--- Testing DataContainer Class ---")

local container = simple.DataContainer.new()
assert_equal(container:getText(), "Empty", "DataContainer default text")
assert_equal(container:getNumberCount(), 0, "DataContainer default number count")

-- 测试文本操作
container:setText("Test Data")
assert_equal(container:getText(), "Test Data", "DataContainer setText/getText")

-- 测试数字操作
container:addNumber(10)
container:addNumber(20)
container:addNumber(30)
assert_equal(container:getNumberCount(), 3, "DataContainer number count after adding")
assert_equal(container:getNumberAt(0), 10, "DataContainer getNumberAt(0)")
assert_equal(container:getNumberAt(1), 20, "DataContainer getNumberAt(1)")
assert_equal(container:getNumberAt(2), 30, "DataContainer getNumberAt(2)")
assert_equal(container:getNumberAt(5), 0, "DataContainer getNumberAt(invalid index)")

-- 测试toString
local str_repr = container:toString()
assert_true(string.find(str_repr, "Test Data") ~= nil, "DataContainer toString contains text")
assert_true(string.find(str_repr, "10") ~= nil, "DataContainer toString contains numbers")

-- 测试清理
container:clearNumbers()
assert_equal(container:getNumberCount(), 0, "DataContainer clearNumbers")

-- 测试StringUtils静态类
print("\n--- Testing StringUtils Static Class ---")

local test_string = "Hello World"

-- 测试大小写转换
assert_equal(simple.StringUtils.toUpperCase(test_string), "HELLO WORLD", "StringUtils.toUpperCase")
assert_equal(simple.StringUtils.toLowerCase(test_string), "hello world", "StringUtils.toLowerCase")

-- 测试反转
assert_equal(simple.StringUtils.reverse(test_string), "dlroW olleH", "StringUtils.reverse")

-- 测试长度
assert_equal(simple.StringUtils.length(test_string), 11, "StringUtils.length")

-- 测试包含检查
assert_true(simple.StringUtils.contains(test_string, "World"), "StringUtils.contains positive")
assert_false(simple.StringUtils.contains(test_string, "xyz"), "StringUtils.contains negative")

-- 测试前缀后缀检查
assert_true(simple.StringUtils.startsWith(test_string, "Hello"), "StringUtils.startsWith positive")
assert_false(simple.StringUtils.startsWith(test_string, "World"), "StringUtils.startsWith negative")
assert_true(simple.StringUtils.endsWith(test_string, "World"), "StringUtils.endsWith positive")
assert_false(simple.StringUtils.endsWith(test_string, "Hello"), "StringUtils.endsWith negative")

-- 测试分割（注意：这里需要Lua绑定支持vector<string>）
-- local parts = simple.StringUtils.split("apple,banana,cherry", ",")
-- assert_equal(#parts, 3, "StringUtils.split result count")
-- assert_equal(parts[1], "apple", "StringUtils.split first part")
-- assert_equal(parts[2], "banana", "StringUtils.split second part")
-- assert_equal(parts[3], "cherry", "StringUtils.split third part")

-- 测试连接
-- local joined = simple.StringUtils.join({"a", "b", "c"}, "-")
-- assert_equal(joined, "a-b-c", "StringUtils.join")

print("\n--- Testing Edge Cases ---")

-- 测试空字符串
assert_equal(simple.StringUtils.length(""), 0, "Empty string length")
assert_equal(simple.StringUtils.toUpperCase(""), "", "Empty string toUpperCase")

-- 测试特殊字符
local special_str = "Hello@#$%World"
assert_true(simple.StringUtils.contains(special_str, "@#$"), "Special characters contains")

-- 测试数字边界
assert_equal(simple.add(-2147483648, 0), -2147483648, "Integer min value")
assert_equal(simple.add(2147483647, 0), 2147483647, "Integer max value")

-- 测试计算器边界
local calc_test = simple.Calculator.new()
calc_test:setValue(1e10)
assert_equal(calc_test:getValue(), 1e10, "Large number handling")

calc_test:setValue(-1e10)
assert_equal(calc_test:getValue(), -1e10, "Large negative number handling")

print("\n=== Test Summary ===")
print("Tests passed: " .. tests_passed)
print("Tests failed: " .. tests_failed)
print("Total tests: " .. (tests_passed + tests_failed))

if tests_failed == 0 then
    print("🎉 All tests passed!")
    return 0
else
    print("❌ Some tests failed!")
    return 1
end