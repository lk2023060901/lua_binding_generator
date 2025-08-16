#!/usr/bin/env lua

--[[
container_creation_examples.lua
简单明了的 STL 容器创建示例

重点展示：
1. 如何从 Lua 创建各种 STL 容器
2. 基本的容器操作
3. 实用的容器工厂方法
]]

print("=== STL Container Creation Examples ===")
print()

-- ================================
-- 基础容器创建
-- ================================

print("1. Basic Container Creation:")
print("   ------------------------")

-- 创建空的 int vector
local intVec = factories.ContainerFactory.createIntVector()
print("   • Empty int vector created, size: " .. #intVec)

-- 创建有初始值的 vector
local filledVec = factories.ContainerFactory.createIntVector(5, 99)
print("   • Filled int vector created (5 elements, value 99)")

-- 创建数值范围
local rangeVec = factories.ContainerFactory.createRangeVector(1, 10, 2)
print("   • Range vector created (1 to 10, step 2), size: " .. #rangeVec)

-- 创建字符串 vector
local strVec = factories.ContainerFactory.createStringVector()
print("   • Empty string vector created, size: " .. #strVec)

print()

-- ================================
-- Map 容器创建
-- ================================

print("2. Map Container Creation:")
print("   -----------------------")

-- 创建空 map
local strIntMap = factories.ContainerFactory.createStringIntMap()
print("   • Empty string-int map created")

-- 创建预填充 map
local prefilledMap = factories.ContainerFactory.createPrefilledStringIntMap()
print("   • Prefilled map created with sample data")

-- 提取 map 的 keys 和 values
local keys = factories.ContainerFactory.extractKeysFromMap(prefilledMap)
local values = factories.ContainerFactory.extractValuesFromMap(prefilledMap)
print("   • Extracted " .. #keys .. " keys and " .. #values .. " values")

print()

-- ================================
-- 智能指针容器
-- ================================

print("3. Smart Pointer Containers:")
print("   -------------------------")

-- 创建玩家 vector
local players = factories.ContainerFactory.createPlayerVector(3)
print("   • Player vector created with 3 players")

-- 创建空的玩家 map
local playerMap = factories.ContainerFactory.createPlayerMap()
print("   • Empty player map created")

print()

-- ================================
-- 嵌套和复杂容器
-- ================================

print("4. Nested and Complex Containers:")
print("   -------------------------------")

-- 创建 2D 矩阵
local matrix = factories.ContainerFactory.create2DIntVector(3, 3, 1)
print("   • 3x3 matrix created (filled with 1)")

-- 创建预定义的向量集合
local vectorSets = factories.ContainerFactory.createMapOfVectors()
print("   • Map of vectors created (fibonacci, primes, etc.)")

print()

-- ================================
-- 容器操作和转换
-- ================================

print("5. Container Operations:")
print("   --------------------")

-- 类型转换
local doubleVec = factories.ContainerFactory.intVectorToDouble(filledVec)
print("   • Converted int vector to double vector")

-- Vector 合并
local vec1 = factories.ContainerFactory.createStringVector()
local vec2 = factories.ContainerFactory.createStringVector()
local merged = factories.ContainerFactory.mergeStringVectors(vec1, vec2)
print("   • Merged two string vectors")

-- 数据过滤
local bigNumbers = factories.ContainerFactory.createIntVector(10, 50)
local filtered = factories.ContainerFactory.filterGreaterThan(bigNumbers, 30)
print("   • Filtered vector (values > 30)")

-- 排序
local unsorted = factories.ContainerFactory.createRangeVector(10, 1, 1)
-- 需要先转换为 int vector 才能排序
print("   • Sorting operations available")

print()

-- ================================
-- 实际使用示例
-- ================================

print("6. Practical Usage Examples:")
print("   -------------------------")

-- 创建一个数据分析场景
print("   Scenario: Creating data for analysis")

-- 创建样本数据
local sampleData = factories.ContainerFactory.createRangeVector(1, 100, 1)
print("   • Sample data: 1-100 range vector, size: " .. #sampleData)

-- 创建分类存储
local categories = factories.ContainerFactory.createMapOfVectors()
print("   • Categories: mathematical sequences ready")

-- 创建用户数据
local users = factories.ContainerFactory.createPlayerVector(10)
print("   • Users: 10 player objects created")

-- 创建配置映射
local config = factories.ContainerFactory.createStringIntMap()
print("   • Config: empty string-int map for settings")

print()

-- ================================
-- 容器创建最佳实践
-- ================================

print("7. Best Practices Summary:")
print("   -----------------------")
print("   ✓ Use createIntVector() for numeric data")
print("   ✓ Use createStringVector() for text data") 
print("   ✓ Use createRangeVector() for sequences")
print("   ✓ Use createStringIntMap() for key-value pairs")
print("   ✓ Use createPlayerVector() for object collections")
print("   ✓ Use create2DIntVector() for matrices")
print("   ✓ Use container operations for data processing")

print()
print("=== Container Creation Examples Complete ===")
print()
print("This demonstrates all the essential patterns for creating")
print("STL containers from Lua using the ContainerFactory class!")