/**
 * @file memory_allocator.h
 * @brief 内存分配器接口定义
 */

#pragma once

#include <cstddef>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <algorithm>

namespace lua_runtime {

/**
 * @brief 内存分配器基础接口
 * 
 * 提供简洁的内存分配接口，用户可以实现自定义分配器
 * 来控制Lua运行时的内存管理策略
 */
class MemoryAllocator {
public:
    virtual ~MemoryAllocator() = default;
    
    /**
     * @brief 分配指定大小的内存
     * @param size 要分配的字节数
     * @param alignment 内存对齐要求，默认为指针大小
     * @return 分配的内存指针，失败时返回nullptr
     */
    virtual void* allocate(size_t size, size_t alignment = sizeof(void*)) = 0;
    
    /**
     * @brief 释放之前分配的内存
     * @param ptr 要释放的内存指针
     * @param size 内存块的大小（提示信息，某些分配器可能需要）
     */
    virtual void deallocate(void* ptr, size_t size) = 0;
    
    /**
     * @brief 重新分配内存（可选实现）
     * @param ptr 原内存指针
     * @param old_size 原内存大小
     * @param new_size 新的内存大小
     * @return 新的内存指针，失败时返回nullptr
     */
    virtual void* reallocate(void* ptr, size_t old_size, size_t new_size) {
        if (new_size == 0) {
            deallocate(ptr, old_size);
            return nullptr;
        }
        
        void* new_ptr = allocate(new_size);
        if (new_ptr && ptr) {
            std::memcpy(new_ptr, ptr, std::min(old_size, new_size));
            deallocate(ptr, old_size);
        }
        return new_ptr;
    }
    
    /**
     * @brief 获取总分配字节数（可选实现）
     * @return 总分配字节数，如果不支持统计则返回0
     */
    virtual size_t getTotalAllocated() const { return 0; }
    
    /**
     * @brief 获取分配次数（可选实现）
     * @return 分配次数，如果不支持统计则返回0
     */
    virtual size_t getAllocationCount() const { return 0; }
    
    /**
     * @brief 重置统计信息（可选实现）
     */
    virtual void resetStatistics() {}
};

/**
 * @brief 默认内存分配器实现
 * 
 * 使用标准C库的malloc/free函数进行内存分配
 */
class DefaultAllocator : public MemoryAllocator {
public:
    void* allocate(size_t size, size_t alignment = sizeof(void*)) override {
        // 使用aligned_alloc如果可用，否则回退到malloc
        #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
            return std::aligned_alloc(alignment, size);
        #else
            // 简单的对齐实现
            if (alignment <= sizeof(void*)) {
                return std::malloc(size);
            }
            
            // 手动对齐
            void* ptr = std::malloc(size + alignment - 1 + sizeof(void*));
            if (!ptr) return nullptr;
            
            void* aligned = reinterpret_cast<void*>(
                (reinterpret_cast<uintptr_t>(ptr) + sizeof(void*) + alignment - 1) & ~(alignment - 1)
            );
            
            // 存储原始指针用于释放
            reinterpret_cast<void**>(aligned)[-1] = ptr;
            return aligned;
        #endif
    }
    
    void deallocate(void* ptr, size_t size) override {
        if (!ptr) return;
        
        #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
            std::free(ptr);
        #else
            // 检查是否是手动对齐的指针
            void* original = reinterpret_cast<void**>(ptr)[-1];
            if (original) {
                std::free(original);
            } else {
                std::free(ptr);
            }
        #endif
    }
};

} // namespace lua_runtime