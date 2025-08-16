--[[
test_framework_standalone.lua
独立的测试框架验证脚本

这个脚本验证测试框架本身是否正常工作，不依赖实际的绑定代码
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

function assert_near(actual, expected, tolerance, message)
    if math.abs(actual - expected) <= tolerance then
        tests_passed = tests_passed + 1
        print("✓ PASS: " .. (message or "test"))
    else
        tests_failed = tests_failed + 1
        print("✗ FAIL: " .. (message or "test") .. " - Expected: " .. tostring(expected) .. "±" .. tostring(tolerance) .. ", Got: " .. tostring(actual))
    end
end

function safe_call(func, error_message)
    local success, result = pcall(func)
    if success then
        tests_passed = tests_passed + 1
        print("✓ PASS: " .. (error_message or "safe call"))
        return result
    else
        tests_failed = tests_failed + 1
        print("✗ FAIL: " .. (error_message or "safe call") .. " - Error: " .. tostring(result))
        return nil
    end
end

print("=== Test Framework Validation ===")
print("Testing the testing framework itself...")
print()

-- 测试基本的断言函数
print("--- Testing Basic Assertions ---")
assert_equal(2 + 2, 4, "Basic math addition")
assert_equal("hello", "hello", "String equality")
assert_true(true, "True condition")
assert_false(false, "False condition")

-- 测试数值近似比较
print("\n--- Testing Numeric Assertions ---")
assert_near(3.14159, 3.14159, 1e-5, "Exact match within tolerance")
assert_near(3.14159, 3.14160, 1e-4, "Close match within tolerance")
assert_near(math.pi, 3.141592653589793, 1e-10, "Pi constant comparison")

-- 测试安全调用
print("\n--- Testing Safe Call Mechanism ---")
safe_call(function()
    return 42
end, "Safe function call")

safe_call(function()
    local result = 10 / 2
    assert_equal(result, 5, "Division in safe call")
    return result
end, "Safe call with assertion")

-- 测试预期失败的情况（这些应该失败，用来验证错误检测）
print("\n--- Testing Error Detection ---")
print("(The following tests are expected to fail to validate error detection)")

assert_equal(2 + 2, 5, "Expected failure: wrong math")
assert_true(false, "Expected failure: false as true")

safe_call(function()
    error("Intentional error for testing")
end, "Expected failure: safe call with error")

-- 生成测试报告
print("\n" .. string.rep("=", 50))
print("TEST FRAMEWORK VALIDATION SUMMARY")
print(string.rep("=", 50))

local total_tests = tests_passed + tests_failed
local success_rate = 0
if total_tests > 0 then
    success_rate = (tests_passed / total_tests) * 100
end

print(string.format("Total tests: %d", total_tests))
print(string.format("Passed: %d", tests_passed))
print(string.format("Failed: %d", tests_failed))
print(string.format("Success rate: %.1f%%", success_rate))

-- 说明预期结果
print()
print("📋 Expected Results:")
print("  - Should have some passing tests (basic assertions)")
print("  - Should have 3 intentionally failing tests")
print("  - This validates that both success and failure detection work correctly")

if tests_passed > 0 and tests_failed == 3 then
    print()
    print("🎉 TEST FRAMEWORK IS WORKING CORRECTLY!")
    print("Ready to test actual Lua bindings when they are generated and integrated.")
    return 0
else
    print()
    print("⚠️  Test framework validation results unexpected.")
    print("Expected: some passes + exactly 3 failures")
    print(string.format("Got: %d passes + %d failures", tests_passed, tests_failed))
    return 1
end