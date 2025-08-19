#pragma once

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/Attr.h>
#include <clang/Basic/SourceLocation.h>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <memory>
#include <fstream>
#include <iomanip>
#include <chrono>

namespace lua_binding_generator {

/**
 * @brief 导出信息结构
 * 
 * 存储从 AST 中提取的导出信息，包括类型、名称、位置和属性等
 */
struct ExportInfo {
    /// 导出类型枚举
    enum class Type {
        Class,              ///< 类
        Method,             ///< 方法
        StaticMethod,       ///< 静态方法
        Constructor,        ///< 构造函数
        Property,           ///< 属性
        Function,           ///< 全局函数
        Enum,               ///< 枚举
        Constant,           ///< 常量
        Namespace,          ///< 命名空间
        Module,             ///< 模块
        Operator,           ///< 操作符重载
        TypeConverter,      ///< 类型转换器
        Inherit,            ///< 继承关系
        STLContainer        ///< STL容器
    };
    
    /// 属性访问类型
    enum class AccessType {
        None,               ///< 无访问控制
        ReadOnly,           ///< 只读
        ReadWrite,          ///< 读写
        WriteOnly           ///< 只写
    };
    
    Type type;                                  ///< 导出类型
    std::string name;                          ///< 名称
    std::string lua_name;                      ///< Lua 中的名称
    std::string qualified_name;                ///< 完全限定名
    std::string source_location;               ///< 源码位置
    std::string file_path;                     ///< 文件路径
    std::string source_file;                   ///< 源文件路径（别名）
    std::map<std::string, std::string> attributes;  ///< 属性映射
    
    // 类型特定信息
    std::string export_type;                   ///< 导出类型字符串
    std::string type_name;                     ///< 类型名称
    std::string return_type;                   ///< 返回类型（方法/函数）
    std::vector<std::string> parameter_types; ///< 参数类型列表
    std::vector<std::string> parameter_names; ///< 参数名称列表
    AccessType access_type = AccessType::None; ///< 访问类型
    std::string property_access;               ///< 属性访问类型字符串 (readonly, readwrite)
    bool is_static = false;                    ///< 是否为静态
    bool is_const = false;                     ///< 是否为常量方法
    bool is_virtual = false;                   ///< 是否为虚方法
    
    // 继承和关联信息
    std::vector<std::string> base_classes;     ///< 基类列表
    std::string owner_class;                   ///< 所属类
    std::string parent_class;                  ///< 父类名称
    std::string namespace_name;                ///< 命名空间
    std::string module_name;                   ///< 模块名
    
    ExportInfo() = default;
    ExportInfo(Type t, const std::string& n) : type(t), name(n), lua_name(n) {
        // 设置source_file为file_path的别名
        source_file = file_path;
        // 设置parent_class为owner_class的别名
        parent_class = owner_class;
    }
};

/**
 * @brief Lua AST 访问器
 * 
 * 继承自 Clang 的 RecursiveASTVisitor，用于遍历 AST 并提取
 * 标记了 EXPORT_LUA_* 宏的 C++ 代码元素信息
 */
class LuaASTVisitor : public clang::RecursiveASTVisitor<LuaASTVisitor> {
public:
    /**
     * @brief 构造函数
     * @param context AST 上下文
     */
    explicit LuaASTVisitor(clang::ASTContext* context);
    
    /**
     * @brief 析构函数
     */
    ~LuaASTVisitor() = default;
    
    // AST 访问器方法
    
    /**
     * @brief 访问 C++ 类/结构体声明
     * @param decl 类声明节点
     * @return true 继续遍历子节点，false 停止遍历
     */
    bool VisitCXXRecordDecl(clang::CXXRecordDecl* decl);
    
    /**
     * @brief 访问 C++ 方法声明
     * @param decl 方法声明节点
     * @return true 继续遍历子节点，false 停止遍历
     */
    bool VisitCXXMethodDecl(clang::CXXMethodDecl* decl);
    
    /**
     * @brief 访问构造函数声明
     * @param decl 构造函数声明节点
     * @return true 继续遍历子节点，false 停止遍历
     */
    bool VisitCXXConstructorDecl(clang::CXXConstructorDecl* decl);
    
    /**
     * @brief 访问字段声明
     * @param decl 字段声明节点
     * @return true 继续遍历子节点，false 停止遍历
     */
    bool VisitFieldDecl(clang::FieldDecl* decl);
    
    /**
     * @brief 访问函数声明
     * @param decl 函数声明节点
     * @return true 继续遍历子节点，false 停止遍历
     */
    bool VisitFunctionDecl(clang::FunctionDecl* decl);
    
    /**
     * @brief 访问枚举声明
     * @param decl 枚举声明节点
     * @return true 继续遍历子节点，false 停止遍历
     */
    bool VisitEnumDecl(clang::EnumDecl* decl);
    
    /**
     * @brief 访问变量声明
     * @param decl 变量声明节点
     * @return true 继续遍历子节点，false 停止遍历
     */
    bool VisitVarDecl(clang::VarDecl* decl);
    
    /**
     * @brief 访问命名空间声明
     * @param decl 命名空间声明节点
     * @return true 继续遍历子节点，false 停止遍历
     */
    bool VisitNamespaceDecl(clang::NamespaceDecl* decl);
    
    /**
     * @brief 处理容器导出标记变量
     * @param decl 变量声明
     * @param export_type 导出注解类型
     * @return true 继续遍历，false 停止遍历
     */
    bool ProcessContainerExport(clang::VarDecl* decl, const std::string& export_type);
    
    // 访问器结果获取
    
    /**
     * @brief 获取提取的导出项列表
     * @return 导出信息向量的常量引用
     */
    const std::vector<ExportInfo>& GetExportedItems() const;
    
    /**
     * @brief 清空已提取的导出项
     */
    void ClearExportedItems();
    
    /**
     * @brief 获取处理的文件数量
     * @return 文件数量
     */
    size_t GetProcessedFileCount() const;
    
    /**
     * @brief 获取错误信息列表
     * @return 错误信息向量的常量引用
     */
    const std::vector<std::string>& GetErrors() const;
    
    /**
     * @brief 初始化日志系统
     * @param debug_log_path 调试日志文件路径
     * @param stats_log_path 统计日志文件路径
     */
    void InitializeLogging(const std::string& debug_log_path, const std::string& stats_log_path);
    
    /**
     * @brief 生成统计报告
     */
    void GenerateStatisticsReport() const;
    
    /**
     * @brief 关闭日志系统
     */
    void CloseLogging();

private:
    clang::ASTContext* context_;                ///< AST 上下文
    std::vector<ExportInfo> exported_items_;   ///< 导出项列表
    std::vector<std::string> errors_;          ///< 错误信息列表
    size_t processed_files_;                   ///< 已处理文件数
    
    // 日志和统计相关
    mutable std::ofstream debug_log_;          ///< 调试日志文件流
    mutable std::ofstream stats_log_;          ///< 统计日志文件流
    std::map<std::string, int> type_counts_;   ///< 类型统计计数器
    
    // 内部辅助方法
    
    /**
     * @brief 检查声明是否有 Lua 导出注解
     * @param decl 声明节点
     * @param export_type 输出的导出类型字符串
     * @return true 如果有导出注解，false 否则
     */
    bool HasLuaExportAnnotation(const clang::Decl* decl, std::string& export_type);
    
    /**
     * @brief 解析注解属性
     * @param annotation 注解字符串
     * @return 属性映射
     */
    std::map<std::string, std::string> ParseAnnotationAttributes(const std::string& annotation);
    
    /**
     * @brief 提取类型信息
     * @param type Clang 类型
     * @return 类型字符串
     */
    std::string ExtractTypeInfo(const clang::QualType& type);
    
    /**
     * @brief 提取参数信息
     * @param function_decl 函数声明
     * @param param_types 输出参数类型列表
     * @param param_names 输出参数名称列表
     */
    void ExtractParameterInfo(const clang::FunctionDecl* function_decl,
                             std::vector<std::string>& param_types,
                             std::vector<std::string>& param_names);
    
    /**
     * @brief 获取声明的完全限定名
     * @param decl 声明节点
     * @return 完全限定名字符串
     */
    std::string GetQualifiedName(const clang::NamedDecl* decl);
    
    /**
     * @brief 获取源码位置字符串
     * @param loc 源码位置
     * @return 位置字符串
     */
    std::string GetSourceLocationString(const clang::SourceLocation& loc);
    
    /**
     * @brief 获取文件路径
     * @param loc 源码位置
     * @return 文件路径字符串
     */
    std::string GetFilePath(const clang::SourceLocation& loc);
    
    /**
     * @brief 确定访问类型
     * @param attr_map 属性映射
     * @return 访问类型枚举
     */
    ExportInfo::AccessType DetermineAccessType(const std::map<std::string, std::string>& attr_map);
    
    /**
     * @brief 提取基类信息
     * @param record_decl 类声明
     * @return 基类名称列表
     */
    std::vector<std::string> ExtractBaseClasses(const clang::CXXRecordDecl* record_decl);
    
    /**
     * @brief 检查是否应该忽略此声明
     * @param decl 声明节点
     * @return true 如果应该忽略，false 否则
     */
    bool ShouldIgnoreDeclaration(const clang::Decl* decl);
    
    /**
     * @brief 记录错误信息
     * @param error 错误描述
     * @param loc 源码位置（可选）
     */
    void RecordError(const std::string& error, const clang::SourceLocation& loc = clang::SourceLocation{});
    
    /**
     * @brief 验证导出信息
     * @param info 导出信息
     * @return true 如果有效，false 否则
     */
    bool ValidateExportInfo(const ExportInfo& info);
    
    /**
     * @brief 自动提取类的所有公共成员
     * @param record_decl 类声明节点
     * @param class_type 类的导出类型 ("class", "static_class", "singleton")
     */
    void ExtractClassMembers(const clang::CXXRecordDecl* record_decl, const std::string& class_type = "class");
    
    /**
     * @brief 提取继承的方法（如果基类未导出到Lua）
     * @param record_decl 类声明节点
     * @param class_name 当前类名
     * @param existing_members 已存在的成员集合（用于避免重复）
     */
    void ExtractInheritedMethods(const clang::CXXRecordDecl* record_decl, const std::string& class_name, std::set<std::string>& existing_members);
    
    /**
     * @brief 检查类是否导出到Lua
     * @param record_decl 类声明节点
     * @return 如果类有导出注解则返回true
     */
    bool IsClassExportedToLua(const clang::CXXRecordDecl* record_decl) const;
    
    /**
     * @brief 推导声明所在的命名空间
     * @param decl 声明节点
     * @return 推导出的命名空间名称
     */
    std::string DeduceNamespaceForDecl(const clang::Decl* decl);
    
    // 日志辅助方法
    
    /**
     * @brief 写入调试日志
     * @param level 日志级别
     * @param function 函数名
     * @param message 日志消息
     */
    void WriteDebugLog(const std::string& level, const std::string& function, const std::string& message) const;
    
    /**
     * @brief 记录访问开始
     * @param visitor_name 访问器名称
     * @param decl_name 声明名称
     * @param source_loc 源码位置
     */
    void LogVisitStart(const std::string& visitor_name, const std::string& decl_name, const std::string& source_loc) const;
    
    /**
     * @brief 记录访问结束
     * @param visitor_name 访问器名称
     * @param success 是否成功
     * @param reason 原因
     */
    void LogVisitEnd(const std::string& visitor_name, bool success, const std::string& reason = "") const;
    
    /**
     * @brief 更新类型统计
     * @param export_type 导出类型
     */
    void UpdateTypeStatistics(const std::string& export_type);
    
    /**
     * @brief 获取当前时间戳
     * @return 格式化的时间戳字符串
     */
    std::string GetTimestamp() const;
    
    // 智能容器命名辅助方法
    
    /**
     * @brief 将字符串首字母大写
     * @param str 输入字符串
     * @return 首字母大写的字符串
     */
    std::string CapitalizeFirst(const std::string& str);
    
    /**
     * @brief 将类型名称转换为友好的 Lua 名称
     * @param type_name 类型名称（如 std::string, demo::Player）
     * @return 友好的名称（如 StdString, DemoPlayer）
     */
    std::string ConvertTypeToFriendlyName(const std::string& type_name);
    
    /**
     * @brief 生成 Vector 容器的 Lua 名称
     * @param element_type 元素类型
     * @return Vector 容器名称（如 IntVector, StdStringVector）
     */
    std::string GenerateVectorName(const std::string& element_type);
    
    /**
     * @brief 生成 Map 容器的 Lua 名称
     * @param key_value_types 键值类型对，格式：KeyType,ValueType
     * @return Map 容器名称（如 IntStdStringMap）
     */
    std::string GenerateMapName(const std::string& key_value_types);
    
    /**
     * @brief 生成通用容器的 Lua 名称（后备方案）
     * @param container_type 容器类型
     * @param type_info 类型信息
     * @return 通用容器名称
     */
    std::string GenerateGenericContainerName(const std::string& container_type, const std::string& type_info);
};

} // namespace lua_binding_generator