/**
 * @file advanced_allocators.h
 * @brief 高级内存分配器实现
 * 
 * 提供多种专用的内存分配器，用于不同场景下的性能优化
 */

#pragma once

#include "memory_allocator.h"
#include "runtime_logger.h"
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <unordered_map>
#include <chrono>

// 平台特定头文件
#ifdef _WIN32
    #include <windows.h>
#elif defined(__unix__) || defined(__APPLE__)
    #include <sys/mman.h>
    #include <unistd.h>
#endif

namespace lua_runtime {

/**
 * @brief 堆栈分配器
 * 
 * 适用于临时对象的快速分配，采用LIFO（后进先出）策略
 * 内存在作用域结束时自动释放
 */
class StackAllocator : public MemoryAllocator {
private:
    struct StackFrame {
        size_t offset;
        size_t size;
        const char* tag;
    };
    
    uint8_t* memory_pool_;
    size_t pool_size_;
    size_t current_offset_;
    std::vector<StackFrame> stack_frames_;
    mutable std::mutex mutex_;
    
    // 统计信息
    size_t total_allocated_;
    size_t allocation_count_;
    size_t peak_usage_;
    
public:
    explicit StackAllocator(size_t pool_size = 1024 * 1024) // 默认1MB
        : memory_pool_(nullptr), pool_size_(pool_size), current_offset_(0)
        , total_allocated_(0), allocation_count_(0), peak_usage_(0) {
        
        memory_pool_ = static_cast<uint8_t*>(std::malloc(pool_size_));
        if (!memory_pool_) {
            throw std::bad_alloc();
        }
        
        stack_frames_.reserve(1000); // 预分配栈帧
        LUA_RUNTIME_LOG_DEBUG("StackAllocator 创建，池大小: %zu bytes", pool_size_);
    }
    
    ~StackAllocator() override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (memory_pool_) {
            std::free(memory_pool_);
        }
        LUA_RUNTIME_LOG_DEBUG("StackAllocator 销毁，峰值使用: %zu bytes", peak_usage_);
    }
    
    void* allocate(size_t size, size_t alignment = sizeof(void*)) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 计算对齐后的偏移
        size_t aligned_offset = (current_offset_ + alignment - 1) & ~(alignment - 1);
        
        if (aligned_offset + size > pool_size_) {
            LUA_RUNTIME_LOG_ERROR("StackAllocator 内存不足: 需要 %zu, 剩余 %zu", 
                                size, pool_size_ - aligned_offset);
            return nullptr;
        }
        
        void* ptr = memory_pool_ + aligned_offset;
        
        // 记录栈帧
        stack_frames_.push_back({aligned_offset, size, "stack_alloc"});
        current_offset_ = aligned_offset + size;
        
        // 更新统计
        total_allocated_ += size;
        allocation_count_++;
        peak_usage_ = std::max(peak_usage_, current_offset_);
        
        LUA_RUNTIME_LOG_TRACE("StackAllocator 分配: %zu bytes at offset %zu", size, aligned_offset);
        return ptr;
    }
    
    void deallocate(void* ptr, size_t size) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!ptr || stack_frames_.empty()) {
            return;
        }
        
        // 检查是否是最后分配的内存块（LIFO检查）
        const auto& last_frame = stack_frames_.back();
        if (memory_pool_ + last_frame.offset == ptr) {
            current_offset_ = last_frame.offset;
            stack_frames_.pop_back();
            LUA_RUNTIME_LOG_TRACE("StackAllocator 释放: %zu bytes, 新偏移 %zu", size, current_offset_);
        } else {
            LUA_RUNTIME_LOG_WARN("StackAllocator 非LIFO释放尝试，忽略");
        }
    }
    
    /**
     * @brief 创建栈帧标记点
     * @param tag 标记名称
     * @return 栈帧ID
     */
    size_t pushFrame(const char* tag = "anonymous") {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t frame_id = stack_frames_.size();
        stack_frames_.push_back({current_offset_, 0, tag});
        return frame_id;
    }
    
    /**
     * @brief 回退到指定栈帧，释放之后的所有分配
     * @param frame_id 栈帧ID
     */
    void popFrame(size_t frame_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (frame_id < stack_frames_.size()) {
            current_offset_ = stack_frames_[frame_id].offset;
            stack_frames_.resize(frame_id);
            LUA_RUNTIME_LOG_DEBUG("StackAllocator 回退到帧 %zu, 偏移 %zu", frame_id, current_offset_);
        }
    }
    
    /**
     * @brief 重置分配器，释放所有内存
     */
    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        current_offset_ = 0;
        stack_frames_.clear();
        LUA_RUNTIME_LOG_DEBUG("StackAllocator 重置");
    }
    
    size_t getTotalAllocated() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return total_allocated_;
    }
    
    size_t getAllocationCount() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return allocation_count_;
    }
    
    void resetStatistics() override {
        std::lock_guard<std::mutex> lock(mutex_);
        total_allocated_ = 0;
        allocation_count_ = 0;
        peak_usage_ = 0;
    }
    
    size_t getCurrentUsage() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_offset_;
    }
    
    size_t getPeakUsage() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return peak_usage_;
    }
    
    double getFragmentation() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pool_size_ == 0) return 0.0;
        return 1.0 - (double)current_offset_ / pool_size_;
    }
};

/**
 * @brief 固定大小块内存池分配器
 * 
 * 专门用于分配固定大小的内存块，具有O(1)的分配和释放时间
 */
class PoolAllocator : public MemoryAllocator {
private:
    struct FreeBlock {
        FreeBlock* next;
    };
    
    uint8_t* memory_pool_;
    size_t pool_size_;
    size_t block_size_;
    size_t block_count_;
    FreeBlock* free_list_;
    mutable std::mutex mutex_;
    
    // 统计信息
    std::atomic<size_t> allocated_blocks_;
    std::atomic<size_t> total_allocations_;
    size_t peak_allocated_;
    
public:
    PoolAllocator(size_t block_size, size_t block_count = 1000)
        : memory_pool_(nullptr), pool_size_(0), block_size_(0), block_count_(block_count)
        , free_list_(nullptr), allocated_blocks_(0), total_allocations_(0), peak_allocated_(0) {
        
        // 确保块大小至少能存放指针
        block_size_ = std::max(block_size, sizeof(FreeBlock*));
        // 对齐到指针大小
        block_size_ = (block_size_ + sizeof(void*) - 1) & ~(sizeof(void*) - 1);
        
        pool_size_ = block_size_ * block_count_;
        memory_pool_ = static_cast<uint8_t*>(std::malloc(pool_size_));
        
        if (!memory_pool_) {
            throw std::bad_alloc();
        }
        
        initializePool();
        LUA_RUNTIME_LOG_DEBUG("PoolAllocator 创建: %zu blocks × %zu bytes = %zu total", 
                            block_count_, block_size_, pool_size_);
    }
    
    ~PoolAllocator() override {
        if (memory_pool_) {
            std::free(memory_pool_);
        }
        LUA_RUNTIME_LOG_DEBUG("PoolAllocator 销毁，峰值使用: %zu blocks", peak_allocated_);
    }
    
    void* allocate(size_t size, size_t alignment = sizeof(void*)) override {
        if (size > block_size_) {
            LUA_RUNTIME_LOG_ERROR("PoolAllocator 请求大小 %zu 超过块大小 %zu", size, block_size_);
            return nullptr;
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!free_list_) {
            LUA_RUNTIME_LOG_ERROR("PoolAllocator 内存池耗尽");
            return nullptr;
        }
        
        void* ptr = free_list_;
        free_list_ = free_list_->next;
        
        size_t current_allocated = ++allocated_blocks_;
        peak_allocated_ = std::max(peak_allocated_, current_allocated);
        ++total_allocations_;
        
        LUA_RUNTIME_LOG_TRACE("PoolAllocator 分配块，当前使用: %zu/%zu", 
                            current_allocated, block_count_);
        return ptr;
    }
    
    void deallocate(void* ptr, size_t size) override {
        if (!ptr) return;
        
        // 验证指针是否在有效范围内
        if (ptr < memory_pool_ || ptr >= memory_pool_ + pool_size_) {
            LUA_RUNTIME_LOG_ERROR("PoolAllocator 无效指针释放尝试");
            return;
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        FreeBlock* block = static_cast<FreeBlock*>(ptr);
        block->next = free_list_;
        free_list_ = block;
        
        --allocated_blocks_;
        LUA_RUNTIME_LOG_TRACE("PoolAllocator 释放块，当前使用: %zu/%zu", 
                            allocated_blocks_.load(), block_count_);
    }
    
    size_t getTotalAllocated() const override {
        return allocated_blocks_.load() * block_size_;
    }
    
    size_t getAllocationCount() const override {
        return total_allocations_.load();
    }
    
    void resetStatistics() override {
        total_allocations_.store(0);
        // 注意：allocated_blocks_ 不重置，因为它反映的是当前使用状态
    }
    
    size_t getBlockSize() const { return block_size_; }
    size_t getBlockCount() const { return block_count_; }
    size_t getAllocatedBlocks() const { return allocated_blocks_.load(); }
    size_t getAvailableBlocks() const { return block_count_ - allocated_blocks_.load(); }
    
    bool isFull() const { return allocated_blocks_.load() >= block_count_; }
    
    double getUtilization() const {
        return (double)allocated_blocks_.load() / block_count_;
    }
    
private:
    void initializePool() {
        // 将所有块链接到自由列表
        free_list_ = reinterpret_cast<FreeBlock*>(memory_pool_);
        FreeBlock* current = free_list_;
        
        for (size_t i = 0; i < block_count_ - 1; ++i) {
            current->next = reinterpret_cast<FreeBlock*>(
                reinterpret_cast<uint8_t*>(current) + block_size_
            );
            current = current->next;
        }
        current->next = nullptr;
    }
};

/**
 * @brief 平台特定的虚拟内存分配器
 * 
 * 利用操作系统的虚拟内存管理功能进行大块内存分配
 */
class VirtualMemoryAllocator : public MemoryAllocator {
private:
    struct MemoryRegion {
        void* address;
        size_t size;
        std::chrono::steady_clock::time_point allocated_time;
    };
    
    std::unordered_map<void*, MemoryRegion> allocated_regions_;
    mutable std::mutex mutex_;
    size_t page_size_;
    
    // 统计信息
    std::atomic<size_t> total_allocated_;
    std::atomic<size_t> allocation_count_;
    size_t peak_allocated_;
    
public:
    VirtualMemoryAllocator() 
        : total_allocated_(0), allocation_count_(0), peak_allocated_(0) {
        
        // 获取系统页大小
#ifdef _WIN32
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        page_size_ = si.dwPageSize;
#else
        page_size_ = static_cast<size_t>(sysconf(_SC_PAGESIZE));
#endif
        
        LUA_RUNTIME_LOG_DEBUG("VirtualMemoryAllocator 创建，页大小: %zu bytes", page_size_);
    }
    
    ~VirtualMemoryAllocator() override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 释放所有分配的区域
        for (const auto& [ptr, region] : allocated_regions_) {
            deallocateRegion(region.address, region.size);
        }
        
        LUA_RUNTIME_LOG_DEBUG("VirtualMemoryAllocator 销毁，峰值使用: %zu bytes", peak_allocated_);
    }
    
    void* allocate(size_t size, size_t alignment = sizeof(void*)) override {
        // 向上舍入到页边界
        size_t aligned_size = ((size + page_size_ - 1) / page_size_) * page_size_;
        
        void* ptr = allocateRegion(aligned_size);
        if (!ptr) {
            LUA_RUNTIME_LOG_ERROR("VirtualMemoryAllocator 分配失败: %zu bytes", aligned_size);
            return nullptr;
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        allocated_regions_[ptr] = {ptr, aligned_size, std::chrono::steady_clock::now()};
        
        size_t current_total = total_allocated_ += aligned_size;
        peak_allocated_ = std::max(peak_allocated_, current_total);
        ++allocation_count_;
        
        LUA_RUNTIME_LOG_DEBUG("VirtualMemoryAllocator 分配: %zu bytes (对齐后 %zu)", size, aligned_size);
        return ptr;
    }
    
    void deallocate(void* ptr, size_t size) override {
        if (!ptr) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = allocated_regions_.find(ptr);
        if (it == allocated_regions_.end()) {
            LUA_RUNTIME_LOG_ERROR("VirtualMemoryAllocator 尝试释放未知指针");
            return;
        }
        
        const auto& region = it->second;
        deallocateRegion(region.address, region.size);
        
        total_allocated_ -= region.size;
        allocated_regions_.erase(it);
        
        LUA_RUNTIME_LOG_DEBUG("VirtualMemoryAllocator 释放: %zu bytes", region.size);
    }
    
    size_t getTotalAllocated() const override {
        return total_allocated_.load();
    }
    
    size_t getAllocationCount() const override {
        return allocation_count_.load();
    }
    
    void resetStatistics() override {
        // 重置累计统计，但保留当前分配状态
        allocation_count_.store(0);
        peak_allocated_ = 0;
        // 注意：total_allocated_ 不重置，因为它反映的是当前实际分配的内存
    }
    
    size_t getPageSize() const { return page_size_; }
    size_t getRegionCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return allocated_regions_.size();
    }
    
private:
    void* allocateRegion(size_t size) {
#ifdef _WIN32
        return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
        void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        return (ptr == MAP_FAILED) ? nullptr : ptr;
#endif
    }
    
    void deallocateRegion(void* ptr, size_t size) {
#ifdef _WIN32
        VirtualFree(ptr, 0, MEM_RELEASE);
#else
        munmap(ptr, size);
#endif
    }
};

/**
 * @brief 分配器工厂类
 * 
 * 提供创建不同类型分配器的工厂方法
 */
class AllocatorFactory {
public:
    /**
     * @brief 创建堆栈分配器
     */
    static std::shared_ptr<StackAllocator> createStackAllocator(size_t pool_size = 1024 * 1024) {
        return std::make_shared<StackAllocator>(pool_size);
    }
    
    /**
     * @brief 创建内存池分配器
     */
    static std::shared_ptr<PoolAllocator> createPoolAllocator(size_t block_size, size_t block_count = 1000) {
        return std::make_shared<PoolAllocator>(block_size, block_count);
    }
    
    /**
     * @brief 创建虚拟内存分配器
     */
    static std::shared_ptr<VirtualMemoryAllocator> createVirtualMemoryAllocator() {
        return std::make_shared<VirtualMemoryAllocator>();
    }
    
    /**
     * @brief 创建默认分配器
     */
    static std::shared_ptr<DefaultAllocator> createDefaultAllocator() {
        return std::make_shared<DefaultAllocator>();
    }
    
    /**
     * @brief 根据使用场景推荐分配器
     */
    static std::shared_ptr<MemoryAllocator> createRecommendedAllocator(const std::string& use_case) {
        if (use_case == "temp_objects" || use_case == "stack") {
            return createStackAllocator();
        } else if (use_case == "fixed_size" || use_case == "pool") {
            return createPoolAllocator(64); // 64字节块，适合小对象
        } else if (use_case == "large_blocks" || use_case == "virtual") {
            return createVirtualMemoryAllocator();
        } else {
            return createDefaultAllocator();
        }
    }
};

} // namespace lua_runtime