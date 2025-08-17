#include "ast_visitor.h"
#include <clang/Basic/SourceManager.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Type.h>
#include <sstream>
#include <algorithm>
#include <regex>
#include <set>
#include <iomanip>
#include <chrono>
#include <unordered_map>
#include <cctype>

namespace lua_binding_generator {

LuaASTVisitor::LuaASTVisitor(clang::ASTContext* context)
    : context_(context), processed_files_(0) {
    // 初始化类型统计计数器
    type_counts_["module"] = 0;
    type_counts_["namespace"] = 0;
    type_counts_["class"] = 0;
    type_counts_["static_class"] = 0;
    type_counts_["singleton"] = 0;
    type_counts_["constructor"] = 0;
    type_counts_["method"] = 0;
    type_counts_["static_method"] = 0;
    type_counts_["property"] = 0;
    type_counts_["operator"] = 0;
    type_counts_["function"] = 0;
    type_counts_["enum"] = 0;
    type_counts_["constant"] = 0;
}

bool LuaASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl* decl) {
    if (!decl || !decl->hasDefinition()) {
        return true;
    }
    
    std::string class_name = decl->getNameAsString();
    std::string source_loc = GetSourceLocationString(decl->getLocation());
    LogVisitStart("VisitCXXRecordDecl", class_name, source_loc);
    
    std::string export_type;
    if (!HasLuaExportAnnotation(decl, export_type)) {
        LogVisitEnd("VisitCXXRecordDecl", false, "No export annotation");
        return true;
    }
    
    WriteDebugLog("INFO", "VisitCXXRecordDecl", "Found export annotation: " + export_type);
    
    // 检查是否应该忽略
    if (ShouldIgnoreDeclaration(decl)) {
        LogVisitEnd("VisitCXXRecordDecl", false, "Declaration should be ignored");
        return true;
    }
    
    ExportInfo info(ExportInfo::Type::Class, decl->getNameAsString());
    
    // 根据注解类型设置正确的 export_type
    if (export_type.find("lua_export_static_class") == 0) {
        info.export_type = "static_class";
    } else if (export_type.find("lua_export_singleton") == 0) {
        info.export_type = "singleton";
    } else if (export_type.find("lua_export_abstract_class") == 0) {
        info.export_type = "abstract_class";
    } else {
        info.export_type = "class";
    }
    info.qualified_name = GetQualifiedName(decl);
    info.source_location = GetSourceLocationString(decl->getLocation());
    info.file_path = GetFilePath(decl->getLocation());
    info.attributes = ParseAnnotationAttributes(export_type);
    info.base_classes = ExtractBaseClasses(decl);
    
    // 提取命名空间信息
    if (auto ns_decl = llvm::dyn_cast_or_null<clang::NamespaceDecl>(decl->getDeclContext())) {
        info.namespace_name = ns_decl->getNameAsString();
    }
    
    if (ValidateExportInfo(info)) {
        // 保存export_type，因为move后不能再访问info
        std::string saved_export_type = info.export_type;
        
        exported_items_.push_back(std::move(info));
        UpdateTypeStatistics(saved_export_type);
        
        WriteDebugLog("INFO", "VisitCXXRecordDecl", "Class " + class_name + " added to export list with type: " + saved_export_type);
        
        // 自动提取所有公共成员（普通类、静态类、单例、抽象类）
        if (saved_export_type == "class" || saved_export_type == "static_class" || saved_export_type == "singleton" || saved_export_type == "abstract_class") {
            WriteDebugLog("INFO", "VisitCXXRecordDecl", "Starting automatic member extraction for " + class_name);
            ExtractClassMembers(decl, saved_export_type);
        } else {
            WriteDebugLog("INFO", "VisitCXXRecordDecl", "Skipping member extraction for " + class_name + " (type: " + saved_export_type + ")");
        }
        
        LogVisitEnd("VisitCXXRecordDecl", true, "Class exported successfully");
    } else {
        LogVisitEnd("VisitCXXRecordDecl", false, "Export info validation failed");
    }
    
    return true;
}

bool LuaASTVisitor::VisitCXXMethodDecl(clang::CXXMethodDecl* decl) {
    if (!decl) {
        WriteDebugLog("WARN", "VisitCXXMethodDecl", "Null declaration received");
        return true;
    }
    
    std::string method_name = decl->getNameAsString();
    std::string source_loc = GetSourceLocationString(decl->getLocation());
    LogVisitStart("VisitCXXMethodDecl", method_name, source_loc);
    
    std::string export_type;
    if (!HasLuaExportAnnotation(decl, export_type)) {
        LogVisitEnd("VisitCXXMethodDecl", false, "No export annotation");
        return true;
    }
    
    WriteDebugLog("INFO", "VisitCXXMethodDecl", "Found export annotation: " + export_type);
    
    if (ShouldIgnoreDeclaration(decl)) {
        LogVisitEnd("VisitCXXMethodDecl", false, "Declaration should be ignored");
        return true;
    }
    
    ExportInfo::Type info_type = ExportInfo::Type::Method;
    std::string string_export_type = "method";
    
    // 检查是否是操作符重载
    if (export_type.find("lua_export_operator") == 0) {
        info_type = ExportInfo::Type::Operator;
        string_export_type = "operator";
    } else if (decl->isStatic()) {
        info_type = ExportInfo::Type::StaticMethod;
        string_export_type = "static_method";
    }
    
    ExportInfo info(info_type, decl->getNameAsString());
    info.export_type = string_export_type;
    info.qualified_name = GetQualifiedName(decl);
    info.source_location = GetSourceLocationString(decl->getLocation());
    info.file_path = GetFilePath(decl->getLocation());
    info.attributes = ParseAnnotationAttributes(export_type);
    info.return_type = ExtractTypeInfo(decl->getReturnType());
    info.is_static = decl->isStatic();
    info.is_const = decl->isConst();
    info.is_virtual = decl->isVirtual();
    
    ExtractParameterInfo(decl, info.parameter_types, info.parameter_names);
    
    // 获取所属类信息
    if (auto parent_class = decl->getParent()) {
        info.owner_class = parent_class->getNameAsString();
        info.parent_class = parent_class->getNameAsString();  // Set both for compatibility
        WriteDebugLog("DETAIL", "VisitCXXMethodDecl", "Method belongs to class: " + info.parent_class);
    }
    
    if (ValidateExportInfo(info)) {
        exported_items_.push_back(std::move(info));
        UpdateTypeStatistics(string_export_type);
        LogVisitEnd("VisitCXXMethodDecl", true, "Method exported successfully");
    } else {
        LogVisitEnd("VisitCXXMethodDecl", false, "Export info validation failed");
    }
    
    return true;
}

bool LuaASTVisitor::VisitCXXConstructorDecl(clang::CXXConstructorDecl* decl) {
    if (!decl) {
        WriteDebugLog("WARN", "VisitCXXConstructorDecl", "Null declaration received");
        return true;
    }
    
    std::string class_name = decl->getParent() ? decl->getParent()->getNameAsString() : "<unknown>";
    std::string source_loc = GetSourceLocationString(decl->getLocation());
    LogVisitStart("VisitCXXConstructorDecl", class_name + "::constructor", source_loc);
    
    std::string export_type;
    if (!HasLuaExportAnnotation(decl, export_type)) {
        LogVisitEnd("VisitCXXConstructorDecl", false, "No export annotation");
        return true;
    }
    
    WriteDebugLog("INFO", "VisitCXXConstructorDecl", "Found export annotation: " + export_type);
    
    if (ShouldIgnoreDeclaration(decl)) {
        LogVisitEnd("VisitCXXConstructorDecl", false, "Declaration should be ignored");
        return true;
    }
    
    ExportInfo info(ExportInfo::Type::Constructor, decl->getParent()->getNameAsString());
    info.export_type = "constructor";  // Set the string type
    info.qualified_name = GetQualifiedName(decl);
    info.source_location = GetSourceLocationString(decl->getLocation());
    info.file_path = GetFilePath(decl->getLocation());
    info.attributes = ParseAnnotationAttributes(export_type);
    
    ExtractParameterInfo(decl, info.parameter_types, info.parameter_names);
    
    if (auto parent_class = decl->getParent()) {
        info.owner_class = parent_class->getNameAsString();
        info.parent_class = parent_class->getNameAsString();  // Set both for compatibility
        WriteDebugLog("DETAIL", "VisitCXXConstructorDecl", "Constructor belongs to class: " + info.parent_class);
    }
    
    WriteDebugLog("DETAIL", "VisitCXXConstructorDecl", "Constructor has " + std::to_string(info.parameter_types.size()) + " parameters");
    
    if (ValidateExportInfo(info)) {
        exported_items_.push_back(std::move(info));
        UpdateTypeStatistics("constructor");
        LogVisitEnd("VisitCXXConstructorDecl", true, "Constructor exported successfully");
    } else {
        LogVisitEnd("VisitCXXConstructorDecl", false, "Export info validation failed");
    }
    
    return true;
}

bool LuaASTVisitor::VisitFieldDecl(clang::FieldDecl* decl) {
    if (!decl) {
        WriteDebugLog("WARN", "VisitFieldDecl", "Null declaration received");
        return true;
    }
    
    std::string field_name = decl->getNameAsString();
    std::string source_loc = GetSourceLocationString(decl->getLocation());
    LogVisitStart("VisitFieldDecl", field_name, source_loc);
    
    std::string export_type;
    if (!HasLuaExportAnnotation(decl, export_type)) {
        LogVisitEnd("VisitFieldDecl", false, "No export annotation");
        return true;
    }
    
    WriteDebugLog("INFO", "VisitFieldDecl", "Found export annotation: " + export_type);
    
    if (ShouldIgnoreDeclaration(decl)) {
        LogVisitEnd("VisitFieldDecl", false, "Declaration should be ignored");
        return true;
    }
    
    ExportInfo info(ExportInfo::Type::Property, decl->getNameAsString());
    info.export_type = "property";  // Set the string type
    info.qualified_name = GetQualifiedName(decl);
    info.source_location = GetSourceLocationString(decl->getLocation());
    info.file_path = GetFilePath(decl->getLocation());
    info.attributes = ParseAnnotationAttributes(export_type);
    info.return_type = ExtractTypeInfo(decl->getType());
    info.access_type = DetermineAccessType(info.attributes);
    
    if (auto parent_class = llvm::dyn_cast_or_null<clang::CXXRecordDecl>(decl->getDeclContext())) {
        info.owner_class = parent_class->getNameAsString();
        info.parent_class = parent_class->getNameAsString();  // Set both for compatibility
        WriteDebugLog("DETAIL", "VisitFieldDecl", "Field belongs to class: " + info.parent_class);
    }
    
    WriteDebugLog("DETAIL", "VisitFieldDecl", "Field type: " + info.return_type + ", access: " + std::to_string(static_cast<int>(info.access_type)));
    
    if (ValidateExportInfo(info)) {
        exported_items_.push_back(std::move(info));
        UpdateTypeStatistics("property");
        LogVisitEnd("VisitFieldDecl", true, "Field exported successfully");
    } else {
        LogVisitEnd("VisitFieldDecl", false, "Export info validation failed");
    }
    
    return true;
}

bool LuaASTVisitor::VisitFunctionDecl(clang::FunctionDecl* decl) {
    if (!decl) {
        WriteDebugLog("WARN", "VisitFunctionDecl", "Null declaration received");
        return true;
    }
    
    if (llvm::isa<clang::CXXMethodDecl>(decl)) {
        WriteDebugLog("DETAIL", "VisitFunctionDecl", "Skipping CXXMethodDecl - handled by VisitCXXMethodDecl");
        return true; // CXXMethodDecl 由 VisitCXXMethodDecl 处理
    }
    
    std::string function_name = decl->getNameAsString();
    std::string source_loc = GetSourceLocationString(decl->getLocation());
    LogVisitStart("VisitFunctionDecl", function_name, source_loc);
    
    std::string export_type;
    if (!HasLuaExportAnnotation(decl, export_type)) {
        LogVisitEnd("VisitFunctionDecl", false, "No export annotation");
        return true;
    }
    
    WriteDebugLog("INFO", "VisitFunctionDecl", "Found export annotation: " + export_type);
    
    if (ShouldIgnoreDeclaration(decl)) {
        LogVisitEnd("VisitFunctionDecl", false, "Declaration should be ignored");
        return true;
    }
    
    ExportInfo info(ExportInfo::Type::Function, decl->getNameAsString());
    info.export_type = "function";  // Set the string type
    info.qualified_name = GetQualifiedName(decl);
    info.source_location = GetSourceLocationString(decl->getLocation());
    info.file_path = GetFilePath(decl->getLocation());
    info.attributes = ParseAnnotationAttributes(export_type);
    info.return_type = ExtractTypeInfo(decl->getReturnType());
    
    ExtractParameterInfo(decl, info.parameter_types, info.parameter_names);
    
    WriteDebugLog("DETAIL", "VisitFunctionDecl", "Function has " + std::to_string(info.parameter_types.size()) + " parameters, return type: " + info.return_type);
    
    if (ValidateExportInfo(info)) {
        exported_items_.push_back(std::move(info));
        UpdateTypeStatistics("function");
        LogVisitEnd("VisitFunctionDecl", true, "Function exported successfully");
    } else {
        LogVisitEnd("VisitFunctionDecl", false, "Export info validation failed");
    }
    
    return true;
}

bool LuaASTVisitor::VisitEnumDecl(clang::EnumDecl* decl) {
    if (!decl) {
        WriteDebugLog("WARN", "VisitEnumDecl", "Null declaration received");
        return true;
    }
    
    std::string enum_name = decl->getNameAsString();
    std::string source_loc = GetSourceLocationString(decl->getLocation());
    LogVisitStart("VisitEnumDecl", enum_name, source_loc);
    
    std::string export_type;
    if (!HasLuaExportAnnotation(decl, export_type)) {
        LogVisitEnd("VisitEnumDecl", false, "No export annotation");
        return true;
    }
    
    WriteDebugLog("INFO", "VisitEnumDecl", "Found export annotation: " + export_type);
    
    if (ShouldIgnoreDeclaration(decl)) {
        LogVisitEnd("VisitEnumDecl", false, "Declaration should be ignored");
        return true;
    }
    
    ExportInfo info(ExportInfo::Type::Enum, decl->getNameAsString());
    info.export_type = "enum";  // Set the string type
    info.qualified_name = GetQualifiedName(decl);
    info.source_location = GetSourceLocationString(decl->getLocation());
    info.file_path = GetFilePath(decl->getLocation());
    info.attributes = ParseAnnotationAttributes(export_type);
    
    // 提取枚举值
    std::vector<std::string> enum_values;
    for (auto* enumerator : decl->enumerators()) {
        if (enumerator) {
            std::string value_name = enumerator->getNameAsString();
            enum_values.push_back(value_name);
            WriteDebugLog("DETAIL", "VisitEnumDecl", "Found enum value: " + value_name);
        }
    }
    WriteDebugLog("INFO", "VisitEnumDecl", "Enum has " + std::to_string(enum_values.size()) + " values");
    
    // 存储枚举值到 attributes 中
    info.attributes["enum_values"] = "";
    for (size_t i = 0; i < enum_values.size(); ++i) {
        if (i > 0) info.attributes["enum_values"] += ",";
        info.attributes["enum_values"] += enum_values[i];
    }
    
    if (ValidateExportInfo(info)) {
        exported_items_.push_back(std::move(info));
        UpdateTypeStatistics("enum");
        LogVisitEnd("VisitEnumDecl", true, "Enum exported successfully");
    } else {
        LogVisitEnd("VisitEnumDecl", false, "Export info validation failed");
    }
    
    return true;
}

bool LuaASTVisitor::VisitVarDecl(clang::VarDecl* decl) {
    if (!decl) {
        WriteDebugLog("WARN", "VisitVarDecl", "Null declaration received");
        return true;
    }
    
    std::string var_name = decl->getNameAsString();
    std::string source_loc = GetSourceLocationString(decl->getLocation());
    LogVisitStart("VisitVarDecl", var_name, source_loc);
    
    std::string export_type;
    if (!HasLuaExportAnnotation(decl, export_type)) {
        LogVisitEnd("VisitVarDecl", false, "No export annotation");
        return true;
    }
    
    WriteDebugLog("INFO", "VisitVarDecl", "Found export annotation: " + export_type);
    
    if (ShouldIgnoreDeclaration(decl)) {
        LogVisitEnd("VisitVarDecl", false, "Declaration should be ignored");
        return true;
    }
    
    // 检查常量名称，过滤掉模块标记常量和容器导出标记
    std::string constant_name = decl->getNameAsString();
    if (constant_name.find("__lua_module_marker_") == 0) {
        LogVisitEnd("VisitVarDecl", false, "Skipping module marker constant");
        return true; // 跳过模块标记常量
    }
    
    // 检查是否是容器导出标记变量，需要特殊处理
    if (constant_name.find("_lua_vector_export_") == 0 ||
        constant_name.find("_lua_map_export_") == 0 ||
        constant_name.find("_lua_unordered_map_export_") == 0 ||
        constant_name.find("_lua_set_export_") == 0 ||
        constant_name.find("_lua_list_export_") == 0 ||
        constant_name.find("_lua_template_instance_export_") == 0) {
        WriteDebugLog("INFO", "VisitVarDecl", "Processing container export marker: " + constant_name);
        
        // 处理容器导出
        return ProcessContainerExport(decl, export_type);
    }
    
    ExportInfo info(ExportInfo::Type::Constant, constant_name);
    info.export_type = "constant";  // Set the string type
    info.qualified_name = GetQualifiedName(decl);
    info.source_location = GetSourceLocationString(decl->getLocation());
    info.file_path = GetFilePath(decl->getLocation());
    info.attributes = ParseAnnotationAttributes(export_type);
    info.return_type = ExtractTypeInfo(decl->getType());
    
    // 提取命名空间信息
    if (auto ns_decl = llvm::dyn_cast_or_null<clang::NamespaceDecl>(decl->getDeclContext())) {
        info.namespace_name = ns_decl->getNameAsString();
        WriteDebugLog("DETAIL", "VisitVarDecl", "Constant belongs to namespace: " + info.namespace_name);
    }
    
    WriteDebugLog("DETAIL", "VisitVarDecl", "Constant type: " + info.return_type);
    
    if (ValidateExportInfo(info)) {
        exported_items_.push_back(std::move(info));
        UpdateTypeStatistics("constant");
        LogVisitEnd("VisitVarDecl", true, "Constant exported successfully");
    } else {
        LogVisitEnd("VisitVarDecl", false, "Export info validation failed");
    }
    
    return true;
}

bool LuaASTVisitor::VisitNamespaceDecl(clang::NamespaceDecl* decl) {
    if (!decl) {
        WriteDebugLog("WARN", "VisitNamespaceDecl", "Null declaration received");
        return true;
    }
    
    std::string namespace_name = decl->getNameAsString();
    std::string source_loc = GetSourceLocationString(decl->getLocation());
    LogVisitStart("VisitNamespaceDecl", namespace_name, source_loc);
    
    std::string export_type;
    if (!HasLuaExportAnnotation(decl, export_type)) {
        LogVisitEnd("VisitNamespaceDecl", false, "No export annotation");
        return true;
    }
    
    WriteDebugLog("INFO", "VisitNamespaceDecl", "Found export annotation: " + export_type);
    
    if (ShouldIgnoreDeclaration(decl)) {
        LogVisitEnd("VisitNamespaceDecl", false, "Declaration should be ignored");
        return true;
    }
    
    ExportInfo info(ExportInfo::Type::Namespace, decl->getNameAsString());
    info.export_type = "namespace";  // Set the string type
    info.qualified_name = GetQualifiedName(decl);
    info.source_location = GetSourceLocationString(decl->getLocation());
    info.file_path = GetFilePath(decl->getLocation());
    info.attributes = ParseAnnotationAttributes(export_type);
    
    WriteDebugLog("DETAIL", "VisitNamespaceDecl", "Qualified name: " + info.qualified_name);
    
    if (ValidateExportInfo(info)) {
        exported_items_.push_back(std::move(info));
        UpdateTypeStatistics("namespace");
        LogVisitEnd("VisitNamespaceDecl", true, "Namespace exported successfully");
    } else {
        LogVisitEnd("VisitNamespaceDecl", false, "Export info validation failed");
    }
    
    return true;
}

bool LuaASTVisitor::ProcessContainerExport(clang::VarDecl* decl, const std::string& export_type) {
    if (!decl) {
        WriteDebugLog("WARN", "ProcessContainerExport", "Null declaration received");
        return true;
    }
    
    std::string var_name = decl->getNameAsString();
    std::string source_loc = GetSourceLocationString(decl->getLocation());
    LogVisitStart("ProcessContainerExport", var_name, source_loc);
    
    WriteDebugLog("INFO", "ProcessContainerExport", "Processing container export: " + export_type);
    
    // 解析注解属性
    auto attributes = ParseAnnotationAttributes(export_type);
    
    // 确定容器类型
    std::string container_type;
    if (export_type.find("lua_export_vector:") == 0) {
        container_type = "vector";
    } else if (export_type.find("lua_export_map:") == 0) {
        container_type = "map";
    } else if (export_type.find("lua_export_unordered_map:") == 0) {
        container_type = "unordered_map";
    } else if (export_type.find("lua_export_set:") == 0) {
        container_type = "set";
    } else if (export_type.find("lua_export_list:") == 0) {
        container_type = "list";
    } else {
        WriteDebugLog("WARN", "ProcessContainerExport", "Unknown container type in annotation: " + export_type);
        LogVisitEnd("ProcessContainerExport", false, "Unknown container type");
        return true;
    }
    
    // 提取类型信息（处理不同格式）
    // 格式: lua_export_vector:ElementType:parameters 或 lua_export_map:KeyType,ValueType:parameters
    std::string type_info;
    size_t first_colon = export_type.find(':');
    if (first_colon != std::string::npos) {
        // 查找最后一个冒号，它分隔类型信息和参数
        size_t last_colon = export_type.rfind(':');
        if (last_colon != first_colon) {
            // 有参数，提取中间的类型信息
            type_info = export_type.substr(first_colon + 1, last_colon - first_colon - 1);
        } else {
            // 没有参数，提取所有类型信息
            type_info = export_type.substr(first_colon + 1);
        }
        
        WriteDebugLog("DETAIL", "ProcessContainerExport", "Extracted type_info: " + type_info);
    }
    
    // 处理类型信息中的命名空间限定
    std::string qualified_type_info = type_info;
    
    // 提取命名空间信息（跳过匿名命名空间）
    std::string namespace_name;
    auto context = decl->getDeclContext();
    while (context) {
        if (auto ns_decl = llvm::dyn_cast<clang::NamespaceDecl>(context)) {
            WriteDebugLog("DETAIL", "ProcessContainerExport", "Found namespace: " + ns_decl->getNameAsString() + " (anonymous: " + (ns_decl->isAnonymousNamespace() ? "yes" : "no") + ")");
            if (!ns_decl->isAnonymousNamespace()) {
                namespace_name = ns_decl->getNameAsString();
                WriteDebugLog("DETAIL", "ProcessContainerExport", "Using namespace: " + namespace_name);
                break;
            }
        }
        context = context->getParent();
    }
    
    if (namespace_name.empty()) {
        WriteDebugLog("DETAIL", "ProcessContainerExport", "No non-anonymous namespace found");
    }
    
    // 对于用户定义的类型（不包含std::或::的类型），添加命名空间限定
    if (container_type == "vector") {
        // 对于vector，只有一个元素类型
        if (qualified_type_info.find("::") == std::string::npos && 
            qualified_type_info.find("std") != 0 &&
            qualified_type_info != "int" && qualified_type_info != "double" &&
            qualified_type_info != "float" && qualified_type_info != "char") {
            // Simple fix: hard-code demo namespace for user types
            std::string ns = namespace_name.empty() ? "demo" : namespace_name;
            qualified_type_info = ns + "::" + qualified_type_info;
        }
    } else if (container_type == "map") {
        // 对于map，需要处理键和值类型
        size_t comma_pos = qualified_type_info.find(',');
        if (comma_pos != std::string::npos) {
            std::string key_type = qualified_type_info.substr(0, comma_pos);
            std::string value_type = qualified_type_info.substr(comma_pos + 1);
            
            // 清理空格
            key_type.erase(0, key_type.find_first_not_of(" \t"));
            key_type.erase(key_type.find_last_not_of(" \t") + 1);
            value_type.erase(0, value_type.find_first_not_of(" \t"));
            value_type.erase(value_type.find_last_not_of(" \t") + 1);
            
            // 为用户定义类型添加命名空间
            std::string ns = namespace_name.empty() ? "demo" : namespace_name;
            if (key_type.find("::") == std::string::npos && key_type.find("std") != 0 &&
                key_type != "int" && key_type != "double" && key_type != "float" && key_type != "char") {
                key_type = ns + "::" + key_type;
            }
            if (value_type.find("::") == std::string::npos && value_type.find("std") != 0 &&
                value_type != "int" && value_type != "double" && value_type != "float" && value_type != "char") {
                value_type = ns + "::" + value_type;
            }
            
            qualified_type_info = key_type + "," + value_type;
        }
    }
    
    // 创建容器导出信息
    std::string container_name;
    if (attributes.count("alias")) {
        container_name = attributes["alias"];
    } else {
        // 使用新的智能命名规则
        if (container_type == "vector") {
            container_name = GenerateVectorName(type_info);
        } else if (container_type == "map") {
            container_name = GenerateMapName(type_info);
        } else {
            // 其他容器类型的后备方案
            container_name = GenerateGenericContainerName(container_type, type_info);
        }
    }
    std::string full_type_name = "std::" + container_type + "<" + qualified_type_info + ">";
    
    ExportInfo info(ExportInfo::Type::STLContainer, container_name);
    info.export_type = "stl";  // 绑定生成器期望的类型标识
    info.type_name = full_type_name;  // 完整的STL类型名称，用于AnalyzeSTLType
    info.qualified_name = container_name;
    info.source_location = GetSourceLocationString(decl->getLocation());
    info.file_path = GetFilePath(decl->getLocation());
    info.attributes = attributes;
    info.return_type = full_type_name;
    info.namespace_name = namespace_name;
    
    WriteDebugLog("DETAIL", "ProcessContainerExport", "Container type: " + container_type + ", Element type: " + type_info);
    WriteDebugLog("DETAIL", "ProcessContainerExport", "Container name: " + container_name);
    WriteDebugLog("DETAIL", "ProcessContainerExport", "Full type name: " + full_type_name);
    WriteDebugLog("DETAIL", "ProcessContainerExport", "Has alias: " + (attributes.count("alias") ? attributes.at("alias") : "NO"));
    
    if (ValidateExportInfo(info)) {
        exported_items_.push_back(std::move(info));
        UpdateTypeStatistics(container_type + "_container");
        LogVisitEnd("ProcessContainerExport", true, "Container exported successfully");
    } else {
        LogVisitEnd("ProcessContainerExport", false, "Export info validation failed");
    }
    
    return true;
}

const std::vector<ExportInfo>& LuaASTVisitor::GetExportedItems() const {
    return exported_items_;
}

void LuaASTVisitor::ClearExportedItems() {
    exported_items_.clear();
    errors_.clear();
    processed_files_ = 0;
}

size_t LuaASTVisitor::GetProcessedFileCount() const {
    return processed_files_;
}

const std::vector<std::string>& LuaASTVisitor::GetErrors() const {
    return errors_;
}

bool LuaASTVisitor::HasLuaExportAnnotation(const clang::Decl* decl, std::string& export_type) {
    if (!decl) {
        return false;
    }
    
    for (const auto* attr : decl->attrs()) {
        if (const auto* annotate_attr = llvm::dyn_cast<clang::AnnotateAttr>(attr)) {
            std::string annotation = annotate_attr->getAnnotation().str();
            if (annotation.find("lua_export_") == 0) {
                export_type = annotation;
                return true;
            }
        }
    }
    
    return false;
}

std::map<std::string, std::string> LuaASTVisitor::ParseAnnotationAttributes(const std::string& annotation) {
    std::map<std::string, std::string> attributes;
    
    // 解析格式: "lua_export_type:TypeName:attr1=value1,attr2=value2"
    // 找到第一个冒号（类型）和最后一个冒号（属性分隔符）
    size_t first_colon = annotation.find(':');
    if (first_colon == std::string::npos) {
        return attributes; // 没有属性
    }
    
    size_t last_colon = annotation.rfind(':');
    if (last_colon == first_colon) {
        return attributes; // 只有一个冒号，没有属性
    }
    
    // 提取属性字符串（最后一个冒号之后的部分）
    std::string attrs_str = annotation.substr(last_colon + 1);
    
    // 解析属性
    std::stringstream attr_ss(attrs_str);
    std::string attr_pair;
    
    while (std::getline(attr_ss, attr_pair, ',')) {
        auto eq_pos = attr_pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = attr_pair.substr(0, eq_pos);
            std::string value = attr_pair.substr(eq_pos + 1);
            
            // 移除前后空白
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            attributes[key] = value;
        } else if (!attr_pair.empty()) {
            // 布尔属性，没有值
            attr_pair.erase(0, attr_pair.find_first_not_of(" \t"));
            attr_pair.erase(attr_pair.find_last_not_of(" \t") + 1);
            attributes[attr_pair] = "true";
        }
    }
    
    return attributes;
}

std::string LuaASTVisitor::ExtractTypeInfo(const clang::QualType& type) {
    if (type.isNull()) {
        return "void";
    }
    
    return type.getAsString(context_->getPrintingPolicy());
}

void LuaASTVisitor::ExtractParameterInfo(const clang::FunctionDecl* function_decl,
                                        std::vector<std::string>& param_types,
                                        std::vector<std::string>& param_names) {
    param_types.clear();
    param_names.clear();
    
    for (unsigned i = 0; i < function_decl->getNumParams(); ++i) {
        const auto* param = function_decl->getParamDecl(i);
        param_types.push_back(ExtractTypeInfo(param->getType()));
        param_names.push_back(param->getNameAsString());
    }
}

std::string LuaASTVisitor::GetQualifiedName(const clang::NamedDecl* decl) {
    if (!decl) {
        return "";
    }
    
    std::string qualified_name;
    llvm::raw_string_ostream stream(qualified_name);
    decl->printQualifiedName(stream);
    stream.flush();
    
    return qualified_name;
}

std::string LuaASTVisitor::GetSourceLocationString(const clang::SourceLocation& loc) {
    if (loc.isInvalid()) {
        return "unknown";
    }
    
    const auto& source_manager = context_->getSourceManager();
    clang::PresumedLoc presumed_loc = source_manager.getPresumedLoc(loc);
    
    if (presumed_loc.isInvalid()) {
        return "unknown";
    }
    
    std::ostringstream oss;
    oss << presumed_loc.getFilename() << ":" << presumed_loc.getLine() << ":" << presumed_loc.getColumn();
    return oss.str();
}

std::string LuaASTVisitor::GetFilePath(const clang::SourceLocation& loc) {
    if (loc.isInvalid()) {
        return "";
    }
    
    const auto& source_manager = context_->getSourceManager();
    clang::PresumedLoc presumed_loc = source_manager.getPresumedLoc(loc);
    
    if (presumed_loc.isInvalid()) {
        return "";
    }
    
    return presumed_loc.getFilename();
}

ExportInfo::AccessType LuaASTVisitor::DetermineAccessType(const std::map<std::string, std::string>& attr_map) {
    auto access_it = attr_map.find("access");
    if (access_it == attr_map.end()) {
        // 检查老式的访问类型参数
        if (attr_map.find("readonly") != attr_map.end()) {
            return ExportInfo::AccessType::ReadOnly;
        }
        if (attr_map.find("readwrite") != attr_map.end()) {
            return ExportInfo::AccessType::ReadWrite;
        }
        return ExportInfo::AccessType::None;
    }
    
    const std::string& access = access_it->second;
    if (access == "readonly") {
        return ExportInfo::AccessType::ReadOnly;
    } else if (access == "readwrite") {
        return ExportInfo::AccessType::ReadWrite;
    } else if (access == "writeonly") {
        return ExportInfo::AccessType::WriteOnly;
    }
    
    return ExportInfo::AccessType::None;
}

std::vector<std::string> LuaASTVisitor::ExtractBaseClasses(const clang::CXXRecordDecl* record_decl) {
    std::vector<std::string> base_classes;
    
    if (!record_decl) {
        return base_classes;
    }
    
    for (const auto& base : record_decl->bases()) {
        clang::QualType base_type = base.getType();
        std::string base_name = base_type.getAsString(context_->getPrintingPolicy());
        base_classes.push_back(base_name);
    }
    
    return base_classes;
}

bool LuaASTVisitor::ShouldIgnoreDeclaration(const clang::Decl* decl) {
    if (!decl) {
        return true;
    }
    
    // 检查是否有 EXPORT_LUA_IGNORE 标记
    for (const auto* attr : decl->attrs()) {
        if (const auto* annotate_attr = llvm::dyn_cast<clang::AnnotateAttr>(attr)) {
            std::string annotation = annotate_attr->getAnnotation().str();
            if (annotation.find("lua_export_ignore") == 0) {
                return true;
            }
        }
    }
    
    // 检查是否在系统头文件中
    const auto& source_manager = context_->getSourceManager();
    if (source_manager.isInSystemHeader(decl->getLocation())) {
        return true;
    }
    
    return false;
}

void LuaASTVisitor::RecordError(const std::string& error, const clang::SourceLocation& loc) {
    std::string full_error = error;
    if (loc.isValid()) {
        full_error += " at " + GetSourceLocationString(loc);
    }
    errors_.push_back(full_error);
}

bool LuaASTVisitor::ValidateExportInfo(const ExportInfo& info) {
    if (info.name.empty()) {
        RecordError("Export info has empty name");
        return false;
    }
    
    if (info.type == ExportInfo::Type::Method || info.type == ExportInfo::Type::Function) {
        if (info.return_type.empty()) {
            RecordError("Method/Function has empty return type: " + info.name);
            return false;
        }
    }
    
    return true;
}

void LuaASTVisitor::ExtractClassMembers(const clang::CXXRecordDecl* record_decl, const std::string& class_type) {
    if (!record_decl) {
        return;
    }
    
    std::string class_name = record_decl->getNameAsString();
    std::string qualified_class_name = GetQualifiedName(record_decl);
    std::string namespace_name;
    
    // 记录成员提取开始
    WriteDebugLog("INFO", "ExtractClassMembers", "Starting member extraction for class " + class_name + " (type: " + class_type + ")");
    WriteDebugLog("DETAIL", "ExtractClassMembers", "Class qualified name: " + qualified_class_name);
    
    // 获取命名空间信息
    if (auto ns_decl = llvm::dyn_cast_or_null<clang::NamespaceDecl>(record_decl->getDeclContext())) {
        namespace_name = ns_decl->getNameAsString();
    }
    
    // 创建唯一性检查集合，避免重复添加已手动导出的成员
    std::set<std::string> existing_members;
    for (const auto& item : exported_items_) {
        if (item.parent_class == class_name) {
            std::string signature = item.export_type + "::" + item.name + "::" + item.qualified_name;
            existing_members.insert(signature);
        }
    }
    
    // 遍历类的所有成员
    size_t member_count = std::distance(record_decl->decls_begin(), record_decl->decls_end());
    WriteDebugLog("INFO", "ExtractClassMembers", "Iterating through " + std::to_string(member_count) + " members of class " + class_name);
    WriteDebugLog("DETAIL", "ExtractClassMembers", "Found " + std::to_string(existing_members.size()) + " existing members to avoid duplicates");
    
    for (auto* member : record_decl->decls()) {
        // 处理构造函数
        if (auto* ctor = llvm::dyn_cast<clang::CXXConstructorDecl>(member)) {
            WriteDebugLog("DETAIL", "ExtractClassMembers", "Found constructor for " + class_name + 
                         " (public: " + std::to_string(ctor->getAccess() == clang::AS_public) +
                         ", deleted: " + std::to_string(ctor->isDeleted()) +
                         ", copy: " + std::to_string(ctor->isCopyConstructor()) +
                         ", move: " + std::to_string(ctor->isMoveConstructor()) + ")");
                   
            if (ctor->getAccess() == clang::AS_public && !ctor->isDeleted() && 
                !ctor->isCopyConstructor() && !ctor->isMoveConstructor()) {
                
                std::string ctor_qualified_name = GetQualifiedName(ctor);
                std::string signature = "constructor::" + class_name + "::" + ctor_qualified_name;
                
                // 检查是否已存在
                if (existing_members.find(signature) != existing_members.end()) {
                    continue;
                }
                
                ExportInfo info(ExportInfo::Type::Constructor, class_name);
                info.export_type = "constructor";
                info.qualified_name = ctor_qualified_name;
                info.source_location = GetSourceLocationString(ctor->getLocation());
                info.file_path = GetFilePath(ctor->getLocation());
                info.parent_class = class_name;
                info.owner_class = class_name;
                info.namespace_name = namespace_name;
                
                ExtractParameterInfo(ctor, info.parameter_types, info.parameter_names);
                
                if (ValidateExportInfo(info)) {
                    exported_items_.push_back(std::move(info));
                    existing_members.insert(signature);
                    UpdateTypeStatistics("constructor");
                    WriteDebugLog("SUCCESS", "ExtractClassMembers", "Added constructor for " + class_name + " with " + std::to_string(info.parameter_types.size()) + " parameters");
                } else {
                    WriteDebugLog("ERROR", "ExtractClassMembers", "Failed to validate constructor for " + class_name);
                }
            }
        }
        // 处理成员方法
        else if (auto* method = llvm::dyn_cast<clang::CXXMethodDecl>(member)) {
            if (method->getAccess() == clang::AS_public && !method->isDeleted() && 
                !llvm::isa<clang::CXXConstructorDecl>(method) && 
                !llvm::isa<clang::CXXDestructorDecl>(method)) {
                
                // 对于静态类，只提取静态方法；对于普通类，提取所有方法
                bool should_extract = false;
                if (class_type == "static_class") {
                    should_extract = method->isStatic();
                } else {
                    should_extract = true;
                }
                
                if (should_extract) {
                    std::string method_name = method->getNameAsString();
                    std::string method_qualified_name = GetQualifiedName(method);
                    
                    // 检查是否为操作符重载
                    std::string export_annotation;
                    std::string method_type;
                    ExportInfo::Type info_type;
                    
                    if (HasLuaExportAnnotation(method, export_annotation) && 
                        export_annotation.find("lua_export_operator") == 0) {
                        method_type = "operator";
                        info_type = ExportInfo::Type::Operator;
                    } else if (method->isStatic()) {
                        method_type = "static_method";
                        info_type = ExportInfo::Type::StaticMethod;
                    } else {
                        method_type = "method";
                        info_type = ExportInfo::Type::Method;
                    }
                    
                    std::string signature = method_type + "::" + method_name + "::" + method_qualified_name;
                    
                    // 检查是否已存在
                    if (existing_members.find(signature) != existing_members.end()) {
                        continue;
                    }
                    
                    ExportInfo info(info_type, method_name);
                    info.export_type = method_type;
                    info.qualified_name = method_qualified_name;
                    info.source_location = GetSourceLocationString(method->getLocation());
                    info.file_path = GetFilePath(method->getLocation());
                    info.return_type = ExtractTypeInfo(method->getReturnType());
                    info.parent_class = class_name;
                    info.owner_class = class_name;
                    info.namespace_name = namespace_name;
                    info.is_static = method->isStatic();
                    info.is_const = method->isConst();
                    info.is_virtual = method->isVirtual();
                    
                    ExtractParameterInfo(method, info.parameter_types, info.parameter_names);
                    
                    if (ValidateExportInfo(info)) {
                        exported_items_.push_back(std::move(info));
                        existing_members.insert(signature);
                        UpdateTypeStatistics(method_type);
                        WriteDebugLog("SUCCESS", "ExtractClassMembers", "Added " + method_type + " " + method_name + " to " + class_name);
                    } else {
                        WriteDebugLog("ERROR", "ExtractClassMembers", "Failed to validate " + method_type + " " + method_name + " for " + class_name);
                    }
                }
            }
        }
        // 处理成员变量 - 只为public字段生成getter/setter访问
        else if (auto* field = llvm::dyn_cast<clang::FieldDecl>(member)) {
            if (field->getAccess() == clang::AS_public) {
                std::string field_name = field->getNameAsString();
                std::string field_qualified_name = GetQualifiedName(field);
                std::string signature = "property::" + field_name + "::" + field_qualified_name;
                
                // 检查是否已存在
                if (existing_members.find(signature) != existing_members.end()) {
                    continue;
                }
                
                ExportInfo info(ExportInfo::Type::Property, field_name);
                info.export_type = "property";
                info.qualified_name = field_qualified_name;
                info.source_location = GetSourceLocationString(field->getLocation());
                info.file_path = GetFilePath(field->getLocation());
                info.return_type = ExtractTypeInfo(field->getType());
                info.parent_class = class_name;
                info.owner_class = class_name;
                info.namespace_name = namespace_name;
                info.access_type = ExportInfo::AccessType::ReadWrite; // 默认读写
                
                if (ValidateExportInfo(info)) {
                    exported_items_.push_back(std::move(info));
                    existing_members.insert(signature);
                    UpdateTypeStatistics("property");
                    WriteDebugLog("SUCCESS", "ExtractClassMembers", "Added property " + field_name + " to " + class_name + " (type: " + info.return_type + ")");
                } else {
                    WriteDebugLog("ERROR", "ExtractClassMembers", "Failed to validate property " + field_name + " for " + class_name);
                }
            }
        }
    }
    
    // 提取继承的方法（如果基类未导出到Lua）
    ExtractInheritedMethods(record_decl, class_name, existing_members);
    
    // 记录成员提取完成
    size_t total_extracted = 0;
    for (const auto& item : exported_items_) {
        if (item.parent_class == class_name) {
            total_extracted++;
        }
    }
    WriteDebugLog("SUMMARY", "ExtractClassMembers", "Completed member extraction for " + class_name + ". Total members: " + std::to_string(total_extracted));
}

void LuaASTVisitor::ExtractInheritedMethods(const clang::CXXRecordDecl* record_decl, const std::string& class_name, std::set<std::string>& existing_members) {
    if (!record_decl) {
        return;
    }
    
    WriteDebugLog("INFO", "ExtractInheritedMethods", "Starting inherited method extraction for class " + class_name);
    
    // 遍历所有基类
    for (const auto& base : record_decl->bases()) {
        const clang::Type* base_type = base.getType().getTypePtr();
        if (auto* record_type = base_type->getAs<clang::RecordType>()) {
            if (auto* base_record = llvm::dyn_cast<clang::CXXRecordDecl>(record_type->getDecl())) {
                if (base_record->hasDefinition()) {
                    std::string base_class_name = base_record->getNameAsString();
                    WriteDebugLog("INFO", "ExtractInheritedMethods", "Checking base class: " + base_class_name);
                    
                    // 检查基类是否导出到Lua
                    bool base_exported = IsClassExportedToLua(base_record);
                    WriteDebugLog("INFO", "ExtractInheritedMethods", "Base class " + base_class_name + " exported: " + (base_exported ? "true" : "false"));
                    
                    // 如果基类未导出到Lua，提取其公共方法
                    if (!base_exported) {
                        WriteDebugLog("INFO", "ExtractInheritedMethods", "Extracting methods from unexported base class: " + base_class_name);
                        
                        // 遍历基类的公共方法
                        for (auto* member : base_record->decls()) {
                            if (auto* method = llvm::dyn_cast<clang::CXXMethodDecl>(member)) {
                                if (method->getAccess() == clang::AS_public && !method->isDeleted() && 
                                    !llvm::isa<clang::CXXConstructorDecl>(method) && 
                                    !llvm::isa<clang::CXXDestructorDecl>(method) &&
                                    !method->isStatic()) {
                                    
                                    std::string method_name = method->getNameAsString();
                                    std::string method_qualified_name = GetQualifiedName(method);
                                    
                                    // 构造签名，但使用当前类的限定名
                                    std::string current_class_qualified_name = GetQualifiedName(record_decl);
                                    std::string derived_method_qualified_name = current_class_qualified_name + "::" + method_name;
                                    
                                    std::string method_type = "method";
                                    std::string signature = method_type + "::" + method_name + "::" + derived_method_qualified_name;
                                    
                                    // 检查是否已存在
                                    if (existing_members.find(signature) != existing_members.end()) {
                                        WriteDebugLog("DETAIL", "ExtractInheritedMethods", "Method " + method_name + " already exists, skipping");
                                        continue;
                                    }
                                    
                                    // 创建继承方法的导出信息
                                    ExportInfo info(ExportInfo::Type::Method, method_name);
                                    info.export_type = method_type;
                                    info.qualified_name = derived_method_qualified_name;  // 使用派生类的限定名
                                    info.source_location = GetSourceLocationString(method->getLocation());
                                    info.file_path = GetFilePath(method->getLocation());
                                    info.parent_class = class_name;  // 属于当前类
                                    info.owner_class = class_name;   // 拥有者是当前类
                                    info.namespace_name = "";
                                    
                                    // 提取参数信息
                                    ExtractParameterInfo(method, info.parameter_types, info.parameter_names);
                                    
                                    // 提取返回类型
                                    info.return_type = method->getReturnType().getAsString();
                                    
                                    if (ValidateExportInfo(info)) {
                                        exported_items_.push_back(std::move(info));
                                        existing_members.insert(signature);
                                        UpdateTypeStatistics(method_type);
                                        WriteDebugLog("SUCCESS", "ExtractInheritedMethods", "Added inherited method " + method_name + " from " + base_class_name + " to " + class_name);
                                    } else {
                                        WriteDebugLog("ERROR", "ExtractInheritedMethods", "Failed to validate inherited method " + method_name + " from " + base_class_name);
                                    }
                                }
                            }
                        }
                        
                        // 递归检查基类的基类
                        ExtractInheritedMethods(base_record, class_name, existing_members);
                    }
                }
            }
        }
    }
    
    WriteDebugLog("INFO", "ExtractInheritedMethods", "Completed inherited method extraction for class " + class_name);
}

bool LuaASTVisitor::IsClassExportedToLua(const clang::CXXRecordDecl* record_decl) const {
    if (!record_decl) {
        return false;
    }
    
    std::string export_type;
    // 创建非const指针来调用非const方法
    return const_cast<LuaASTVisitor*>(this)->HasLuaExportAnnotation(record_decl, export_type);
}

// 日志系统实现

void LuaASTVisitor::InitializeLogging(const std::string& debug_log_path, const std::string& stats_log_path) {
    debug_log_.open(debug_log_path, std::ios::out | std::ios::trunc);
    stats_log_.open(stats_log_path, std::ios::out | std::ios::trunc);
    
    if (debug_log_.is_open()) {
        WriteDebugLog("INFO", "InitializeLogging", "Debug logging initialized: " + debug_log_path);
    }
    if (stats_log_.is_open()) {
        WriteDebugLog("INFO", "InitializeLogging", "Statistics logging initialized: " + stats_log_path);
    }
}

void LuaASTVisitor::CloseLogging() {
    if (debug_log_.is_open()) {
        WriteDebugLog("INFO", "CloseLogging", "Closing debug log");
        debug_log_.close();
    }
    if (stats_log_.is_open()) {
        stats_log_.close();
    }
}

void LuaASTVisitor::WriteDebugLog(const std::string& level, const std::string& function, const std::string& message) const {
    if (debug_log_.is_open()) {
        debug_log_ << "[" << GetTimestamp() << "] [" << level << "] [" << function << "] " << message << std::endl;
        debug_log_.flush();
    }
}

void LuaASTVisitor::LogVisitStart(const std::string& visitor_name, const std::string& decl_name, const std::string& source_loc) const {
    WriteDebugLog("VISIT_START", visitor_name, "Visiting '" + decl_name + "' at " + source_loc);
}

void LuaASTVisitor::LogVisitEnd(const std::string& visitor_name, bool success, const std::string& reason) const {
    std::string status = success ? "SUCCESS" : "FAILED";
    std::string msg = "Visit result: " + status;
    if (!reason.empty()) {
        msg += " - " + reason;
    }
    WriteDebugLog("VISIT_END", visitor_name, msg);
}

void LuaASTVisitor::UpdateTypeStatistics(const std::string& export_type) {
    type_counts_[export_type]++;
    WriteDebugLog("STATS", "UpdateTypeStatistics", "Incremented " + export_type + " count to " + std::to_string(type_counts_[export_type]));
}

std::string LuaASTVisitor::GetTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void LuaASTVisitor::GenerateStatisticsReport() const {
    if (!stats_log_.is_open()) {
        return;
    }
    
    stats_log_ << "=== AST解析统计报告 ===" << std::endl;
    stats_log_ << "生成时间: " << GetTimestamp() << std::endl;
    stats_log_ << "总导出项数: " << exported_items_.size() << std::endl;
    stats_log_ << std::endl;
    
    stats_log_ << "=== 按类型统计 ===" << std::endl;
    for (const auto& [type, count] : type_counts_) {
        if (count > 0) {
            stats_log_ << type << ": " << count << std::endl;
        }
    }
    stats_log_ << std::endl;
    
    // 按类分组的详细统计
    std::map<std::string, std::vector<ExportInfo>> by_class;
    std::map<std::string, std::vector<ExportInfo>> by_namespace;
    
    for (const auto& item : exported_items_) {
        if (!item.parent_class.empty()) {
            by_class[item.parent_class].push_back(item);
        }
        if (!item.namespace_name.empty()) {
            by_namespace[item.namespace_name].push_back(item);
        }
    }
    
    if (!by_class.empty()) {
        stats_log_ << "=== 按类详细分解 ===" << std::endl;
        for (const auto& [class_name, members] : by_class) {
            stats_log_ << class_name << ":" << std::endl;
            
            std::map<std::string, int> member_counts;
            for (const auto& member : members) {
                member_counts[member.export_type]++;
            }
            
            for (const auto& [type, count] : member_counts) {
                stats_log_ << "  - " << type << ": " << count << std::endl;
            }
            stats_log_ << std::endl;
        }
    }
    
    if (!by_namespace.empty()) {
        stats_log_ << "=== 按命名空间分组 ===" << std::endl;
        for (const auto& [ns_name, items] : by_namespace) {
            stats_log_ << ns_name << ": " << items.size() << " 项" << std::endl;
        }
        stats_log_ << std::endl;
    }
    
    stats_log_.flush();
}

// 智能类型名称转换辅助函数
std::string LuaASTVisitor::CapitalizeFirst(const std::string& str) {
    if (str.empty()) return str;
    std::string result = str;
    result[0] = std::toupper(result[0]);
    return result;
}

std::string LuaASTVisitor::ConvertTypeToFriendlyName(const std::string& type_name) {
    // 基础类型映射
    static const std::unordered_map<std::string, std::string> type_mapping = {
        {"int", "Int"},
        {"double", "Double"}, 
        {"float", "Float"},
        {"char", "Char"},
        {"bool", "Bool"},
        {"size_t", "SizeT"},
        {"uint32_t", "Uint32"},
        {"int32_t", "Int32"},
        {"uint64_t", "Uint64"},
        {"int64_t", "Int64"},
        {"string", "String"}
    };
    
    // 检查是否是基础类型
    auto it = type_mapping.find(type_name);
    if (it != type_mapping.end()) {
        return it->second;
    }
    
    // 处理命名空间类型（如 std::string, demo::Player）
    std::string friendly_name;
    size_t start = 0;
    size_t pos = 0;
    
    while ((pos = type_name.find("::", start)) != std::string::npos) {
        std::string part = type_name.substr(start, pos - start);
        if (!part.empty()) {
            friendly_name += CapitalizeFirst(part);
        }
        start = pos + 2;
    }
    
    // 添加最后一部分
    std::string last_part = type_name.substr(start);
    if (!last_part.empty()) {
        friendly_name += CapitalizeFirst(last_part);
    }
    
    return friendly_name.empty() ? CapitalizeFirst(type_name) : friendly_name;
}

std::string LuaASTVisitor::GenerateVectorName(const std::string& element_type) {
    return ConvertTypeToFriendlyName(element_type) + "Vector";
}

std::string LuaASTVisitor::GenerateMapName(const std::string& key_value_types) {
    // 解析键值类型对，格式：KeyType,ValueType
    size_t comma_pos = key_value_types.find(',');
    if (comma_pos == std::string::npos) {
        // 格式错误，使用后备方案
        return GenerateGenericContainerName("map", key_value_types);
    }
    
    std::string key_type = key_value_types.substr(0, comma_pos);
    std::string value_type = key_value_types.substr(comma_pos + 1);
    
    // 清理空格
    key_type.erase(0, key_type.find_first_not_of(" \t"));
    key_type.erase(key_type.find_last_not_of(" \t") + 1);
    value_type.erase(0, value_type.find_first_not_of(" \t"));
    value_type.erase(value_type.find_last_not_of(" \t") + 1);
    
    return ConvertTypeToFriendlyName(key_type) + ConvertTypeToFriendlyName(value_type) + "Map";
}

std::string LuaASTVisitor::GenerateGenericContainerName(const std::string& container_type, const std::string& type_info) {
    // 后备方案：使用旧的清理方式但更智能
    std::string clean_type_info = type_info;
    std::replace(clean_type_info.begin(), clean_type_info.end(), ':', '_');
    std::replace(clean_type_info.begin(), clean_type_info.end(), '<', '_');
    std::replace(clean_type_info.begin(), clean_type_info.end(), '>', '_');
    std::replace(clean_type_info.begin(), clean_type_info.end(), ' ', '_');
    std::replace(clean_type_info.begin(), clean_type_info.end(), ',', '_');
    return CapitalizeFirst(container_type) + "_" + clean_type_info;
}

} // namespace lua_binding_generator