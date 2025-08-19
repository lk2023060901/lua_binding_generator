--[[
    @file operator_overload_test.lua
    @brief 运算符重载测试脚本
    
    测试C++运算符重载在Lua中的绑定，
    使用TestVector2D类验证各种运算符。
]]

print("=== 运算符重载测试 ===")

-- 测试向量类和运算符重载
print("\n--- 向量运算符测试 (TestVector2D) ---")
if TestVector2D then
    print("✅ TestVector2D 类找到")
    
    -- 创建测试向量
    local v1 = TestVector2D(3.0, 4.0)
    local v2 = TestVector2D(1.0, 2.0)
    
    print("   v1 = (" .. v1:getX() .. ", " .. v1:getY() .. ")")
    print("   v2 = (" .. v2:getX() .. ", " .. v2:getY() .. ")")
    
    -- 测试基础方法
    local length = v1:length()
    print("   |v1| =", length)
    assert(math.abs(length - 5.0) < 0.01, "Length of (3,4) should be 5")
    
    local dot_product = v1:dot(v2)
    print("   v1 · v2 =", dot_product)
    assert(dot_product == 11.0, "Dot product should be 11")  -- 3*1 + 4*2 = 11
    
    local cross_product = v1:cross(v2)
    print("   v1 × v2 =", cross_product)
    assert(cross_product == 2.0, "Cross product should be 2")  -- 3*2 - 4*1 = 2
    
    -- 测试加法运算符 (+)
    print("\n--- 加法运算符 (+) ---")
    local v3 = v1 + v2
    print("   v1 + v2 = (" .. v3:getX() .. ", " .. v3:getY() .. ")")
    assert(v3:getX() == 4.0 and v3:getY() == 6.0, "Addition should be (4, 6)")
    
    -- 测试减法运算符 (-)
    print("\n--- 减法运算符 (-) ---")
    local v4 = v1 - v2
    print("   v1 - v2 = (" .. v4:getX() .. ", " .. v4:getY() .. ")")
    assert(v4:getX() == 2.0 and v4:getY() == 2.0, "Subtraction should be (2, 2)")
    
    -- 测试标量乘法运算符 (*)
    print("\n--- 标量乘法运算符 (*) ---")
    local v5 = v1 * 2.0
    print("   v1 * 2.0 = (" .. v5:getX() .. ", " .. v5:getY() .. ")")
    assert(v5:getX() == 6.0 and v5:getY() == 8.0, "Scalar multiplication should be (6, 8)")
    
    -- 测试标量除法运算符 (/)
    print("\n--- 标量除法运算符 (/) ---")
    local v6 = v1 / 2.0
    print("   v1 / 2.0 = (" .. v6:getX() .. ", " .. v6:getY() .. ")")
    assert(v6:getX() == 1.5 and v6:getY() == 2.0, "Scalar division should be (1.5, 2)")
    
    -- 测试相等运算符 (==)
    print("\n--- 相等运算符 (==) ---")
    local v7 = TestVector2D(3.0, 4.0)
    local is_equal = (v1 == v7)
    print("   v1 == v7 =", is_equal)
    assert(is_equal == true, "Vectors with same components should be equal")
    
    local not_equal = (v1 == v2)
    print("   v1 == v2 =", not_equal)
    assert(not_equal == false, "Different vectors should not be equal")
    
    -- 测试不等运算符 (!=)
    print("\n--- 不等运算符 (!=) ---")
    local is_not_equal = (v1 ~= v2)  -- Lua使用 ~= 表示不等
    print("   v1 ~= v2 =", is_not_equal)
    assert(is_not_equal == true, "Different vectors should be not equal")
    
    -- 测试小于运算符 (<) - 基于长度比较
    print("\n--- 小于运算符 (<) ---")
    local v_small = TestVector2D(1.0, 1.0)
    local is_smaller = (v_small < v1)
    print("   (1,1) < (3,4) =", is_smaller)
    assert(is_smaller == true, "Shorter vector should be less than longer vector")
    
    -- 测试索引运算符 ([])
    print("\n--- 索引运算符 ([]) ---")
    local x_component = v1[0]  -- 获取X分量
    local y_component = v1[1]  -- 获取Y分量
    print("   v1[0] =", x_component)
    print("   v1[1] =", y_component)
    assert(x_component == 3.0, "v1[0] should be 3.0")
    assert(y_component == 4.0, "v1[1] should be 4.0")
    
    -- 测试一元负号运算符 (-)
    print("\n--- 一元负号运算符 (-) ---")
    local v_neg = -v1
    print("   -v1 = (" .. v_neg:getX() .. ", " .. v_neg:getY() .. ")")
    assert(v_neg:getX() == -3.0 and v_neg:getY() == -4.0, "Negation should be (-3, -4)")
    
    -- 测试复合赋值运算符 (+=)
    print("\n--- 复合赋值运算符 (+=) ---")
    local v8 = TestVector2D(10.0, 20.0)
    print("   v8 初始值: (" .. v8:getX() .. ", " .. v8:getY() .. ")")
    
    -- 注意：Lua中可能需要通过方法调用来实现复合赋值
    -- 这里我们可能需要检查绑定生成器如何处理这些运算符
    
    -- 测试字符串转换
    print("\n--- 字符串转换 ---")
    local v_str = v1:toString()
    print("   v1.toString() =", v_str)
    assert(type(v_str) == "string", "toString should return a string")
    
    -- 测试归一化
    print("\n--- 向量操作 ---")
    local v_normalized = v1:normalized()
    local norm_length = v_normalized:length()
    print("   归一化向量长度:", norm_length)
    assert(math.abs(norm_length - 1.0) < 0.01, "Normalized vector should have length 1")
    
    -- 测试距离计算
    local distance = v1:distance(v2)
    print("   v1到v2的距离:", distance)
    local expected_dist = math.sqrt((3-1)^2 + (4-2)^2)  -- sqrt(4+4) = sqrt(8)
    assert(math.abs(distance - expected_dist) < 0.01, "Distance calculation should be correct")
    
else
    error("❌ TestVector2D 类未找到")
end

-- 测试零向量和特殊情况
print("\n--- 特殊情况测试 ---")
if TestVector2D then
    local zero_vec = TestVector2D(0.0, 0.0)
    print("   零向量: (" .. zero_vec:getX() .. ", " .. zero_vec:getY() .. ")")
    
    local zero_length = zero_vec:length()
    print("   零向量长度:", zero_length)
    assert(zero_length == 0.0, "Zero vector should have zero length")
    
    -- 测试除零保护
    local div_by_zero = zero_vec / 0.0
    print("   零向量除以0: (" .. div_by_zero:getX() .. ", " .. div_by_zero:getY() .. ")")
    -- 应该返回零向量或处理错误
    
    -- 测试单位向量
    local unit_x = TestVector2D(1.0, 0.0)
    local unit_y = TestVector2D(0.0, 1.0)
    
    local unit_dot = unit_x:dot(unit_y)
    print("   单位向量点积:", unit_dot)
    assert(unit_dot == 0.0, "Perpendicular unit vectors should have zero dot product")
    
    local unit_cross = unit_x:cross(unit_y)
    print("   单位向量叉积:", unit_cross)
    assert(unit_cross == 1.0, "Unit vector cross product should be 1")
end

print("\n🎉 运算符重载测试全部通过！")
return true