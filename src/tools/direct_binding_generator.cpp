/**
 * @file direct_binding_generator.cpp
 * @brief 直接绑定代码生成器实现
 */

#include "direct_binding_generator.h"
#include <algorithm>
#include <regex>
#include <sstream>
#include <iomanip>
#include <set>
#include <iostream>

namespace lua_binding_generator {

// CodeBuilder 实现
DirectBindingGenerator::CodeBuilder::CodeBuilder(int indent_size)
    : indent_level_(0), indent_size_(indent_size) {}

DirectBindingGenerator::CodeBuilder& DirectBindingGenerator::CodeBuilder::AddLine(const std::string& line) {
    content_ << line << "\n";
    return *this;
}

DirectBindingGenerator::CodeBuilder& DirectBindingGenerator::CodeBuilder::AddEmptyLine() {
    content_ << "\n";
    return *this;
}

DirectBindingGenerator::CodeBuilder& DirectBindingGenerator::CodeBuilder::AddIndentedLine(const std::string& line) {
    content_ << GetIndent() << line << "\n";
    return *this;
}

DirectBindingGenerator::CodeBuilder& DirectBindingGenerator::CodeBuilder::IncreaseIndent() {
    indent_level_++;
    return *this;
}

DirectBindingGenerator::CodeBuilder& DirectBindingGenerator::CodeBuilder::DecreaseIndent() {
    if (indent_level_ > 0) {
        indent_level_--;
    }
    return *this;
}

DirectBindingGenerator::CodeBuilder& DirectBindingGenerator::CodeBuilder::AddComment(const std::string& comment) {
    content_ << GetIndent() << "// " << comment << "\n";
    return *this;
}

DirectBindingGenerator::CodeBuilder& DirectBindingGenerator::CodeBuilder::AddBlockComment(const std::vector<std::string>& lines) {
    content_ << GetIndent() << "/*\n";
    for (const auto& line : lines) {
        content_ << GetIndent() << " * " << line << "\n";
    }
    content_ << GetIndent() << " */\n";
    return *this;
}

std::string DirectBindingGenerator::CodeBuilder::Build() const {
    return content_.str();
}

void DirectBindingGenerator::CodeBuilder::Clear() {
    content_.str("");
    content_.clear();
    indent_level_ = 0;
}

std::string DirectBindingGenerator::CodeBuilder::GetIndent() const {
    return std::string(indent_level_ * indent_size_, ' ');
}

// NamespaceManager 实现
std::string DirectBindingGenerator::NamespaceManager::ResolveNamespace(const ExportInfo& info) {
    // 1. 检查是否有明确指定的命名空间
    if (!info.namespace_name.empty() && info.namespace_name != "global") {
        return info.namespace_name;
    }
    
    // 2. 从 qualified_name 中推导命名空间
    if (!info.qualified_name.empty()) {
        // 例如：test_coverage::TestPlayer -> test_coverage
        size_t pos = info.qualified_name.find("::");
        if (pos != std::string::npos) {
            std::string deduced_ns = info.qualified_name.substr(0, pos);
            // 验证这是一个有效的命名空间（不是类名）
            if (deduced_ns != info.name && deduced_ns != info.parent_class) {
                return deduced_ns;
            }
        }
    }
    
    // 3. 从宏参数中推导命名空间
    if (!info.attributes.empty()) {
        auto it = info.attributes.find("namespace");
        if (it != info.attributes.end() && !it->second.empty()) {
            return it->second;
        }
    }
    
    // 4. 从文件路径推导模块命名空间
    if (!info.file_path.empty()) {
        // 如果文件在特定目录中，可能暗示命名空间
        // 例如：examples/complete_test/ -> complete_test
        // 但这个逻辑可能需要更多的上下文信息
    }
    
    // 5. 使用默认全局命名空间
    return "global";
}

std::string DirectBindingGenerator::NamespaceManager::GetNamespaceVariable(const std::string& namespace_name) {
    if (namespace_name == "global") {
        return "lua";
    }
    
    // 检查是否已经创建了变量
    auto it = namespace_vars_.find(namespace_name);
    if (it != namespace_vars_.end()) {
        return it->second;
    }
    
    // 创建新的命名空间变量名
    std::string var_name = namespace_name + "_ns";
    std::replace(var_name.begin(), var_name.end(), '.', '_');
    std::replace(var_name.begin(), var_name.end(), ':', '_');
    
    namespace_vars_[namespace_name] = var_name;
    used_namespaces_.push_back(namespace_name);
    
    return var_name;
}

std::vector<std::string> DirectBindingGenerator::NamespaceManager::GetRequiredNamespaces() const {
    return used_namespaces_;
}

void DirectBindingGenerator::NamespaceManager::Clear() {
    namespace_vars_.clear();
    used_namespaces_.clear();
}

// DirectBindingGenerator 实现
DirectBindingGenerator::DirectBindingGenerator() = default;

void DirectBindingGenerator::SetOptions(const GenerationOptions& options) {
    options_ = options;
}

const DirectBindingGenerator::GenerationOptions& DirectBindingGenerator::GetOptions() const {
    return options_;
}

DirectBindingGenerator::GenerationResult DirectBindingGenerator::GenerateModuleBinding(
    const std::string& module_name,
    const std::vector<ExportInfo>& export_items) {
    
    std::cout << "🔍 [GenerateModuleBinding] 方法开始执行" << std::endl;
    std::cout << "   模块名: \"" << module_name << "\"" << std::endl;
    std::cout << "   导出项数量: " << export_items.size() << std::endl;
    
    GenerationResult result;
    std::cout << "✅ [GenerateModuleBinding] GenerationResult 对象创建成功" << std::endl;
    
    namespace_manager_.Clear();
    std::cout << "✅ [GenerateModuleBinding] 命名空间管理器清理完成" << std::endl;
    
    try {
        std::cout << "🔧 [GenerateModuleBinding] 创建 CodeBuilder (indent_size=" << options_.indent_size << ")" << std::endl;
        CodeBuilder builder(options_.indent_size);
        std::cout << "✅ [GenerateModuleBinding] CodeBuilder 创建成功" << std::endl;
        
        // 1. 生成文件头部
        std::cout << "📝 [GenerateModuleBinding] 步骤1: 生成文件头部" << std::endl;
        try {
            builder.AddLine(GenerateFileHeader(module_name));
            builder.AddEmptyLine();
            std::cout << "✅ [GenerateModuleBinding] 文件头部生成成功" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "❌ [GenerateModuleBinding] 文件头部生成失败: " << e.what() << std::endl;
            throw;
        }
        
        // 2. 生成包含头文件
        std::cout << "📦 [GenerateModuleBinding] 步骤2: 处理包含文件 (generate_includes=" << options_.generate_includes << ")" << std::endl;
        if (options_.generate_includes) {
            try {
                builder.AddLine(GenerateIncludes(export_items));
                builder.AddEmptyLine();
                std::cout << "✅ [GenerateModuleBinding] 包含文件生成成功" << std::endl;
            } catch (const std::exception& e) {
                std::cout << "❌ [GenerateModuleBinding] 包含文件生成失败: " << e.what() << std::endl;
                throw;
            }
        } else {
            std::cout << "⏭️  [GenerateModuleBinding] 跳过包含文件生成" << std::endl;
        }
        
        // 3. 按类型分组导出项
        std::cout << "🗂️  [GenerateModuleBinding] 步骤3: 按类型分组导出项" << std::endl;
        auto grouped_exports = GroupExportsByType(export_items);
        std::cout << "✅ [GenerateModuleBinding] 导出项分组成功，共 " << grouped_exports.size() << " 个类型组" << std::endl;
        
        // 打印各类型的数量
        for (const auto& [type, items] : grouped_exports) {
            std::cout << "   - " << type << ": " << items.size() << " 项" << std::endl;
        }
        
        // 4. 生成绑定代码
        std::cout << "⚙️  [GenerateModuleBinding] 步骤4: 开始生成绑定代码" << std::endl;
        std::string bindings_code;
        CodeBuilder bindings_builder(options_.indent_size);
        std::cout << "✅ [GenerateModuleBinding] 绑定代码 CodeBuilder 创建成功" << std::endl;
        
        // 预先收集所有需要的命名空间
        std::cout << "📂 [GenerateModuleBinding] 步骤4.1: 收集命名空间信息" << std::endl;
        try {
            for (const auto& item : export_items) {
                std::string ns = namespace_manager_.ResolveNamespace(item);
                namespace_manager_.GetNamespaceVariable(ns);
            }
            std::cout << "✅ [GenerateModuleBinding] 命名空间收集完成" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "❌ [GenerateModuleBinding] 命名空间收集失败: " << e.what() << std::endl;
            throw;
        }
        
        // 生成命名空间声明
        std::cout << "📝 [GenerateModuleBinding] 步骤4.2: 生成命名空间声明 (use_namespace_tables=" << options_.use_namespace_tables << ")" << std::endl;
        if (options_.use_namespace_tables) {
            try {
                std::string ns_declarations = GenerateNamespaceDeclarations();
                if (!ns_declarations.empty()) {
                    bindings_builder.AddLine(ns_declarations);
                    bindings_builder.AddEmptyLine();
                    std::cout << "✅ [GenerateModuleBinding] 命名空间声明生成成功" << std::endl;
                } else {
                    std::cout << "ℹ️  [GenerateModuleBinding] 无需生成命名空间声明" << std::endl;
                }
            } catch (const std::exception& e) {
                std::cout << "❌ [GenerateModuleBinding] 命名空间声明生成失败: " << e.what() << std::endl;
                throw;
            }
        } else {
            std::cout << "⏭️  [GenerateModuleBinding] 跳过命名空间声明生成" << std::endl;
        }
        
        // 生成类绑定
        std::cout << "🏗️  [GenerateModuleBinding] 步骤5: 生成类绑定" << std::endl;
        if (grouped_exports.count("class") > 0) {
            std::cout << "   处理 " << grouped_exports.at("class").size() << " 个类绑定" << std::endl;
            bindings_builder.AddComment("Class bindings");
            
            for (size_t i = 0; i < grouped_exports.at("class").size(); ++i) {
                const auto& class_info = grouped_exports.at("class")[i];
                std::cout << "   [" << (i+1) << "/" << grouped_exports.at("class").size() << "] 处理类: " << class_info.name << std::endl;
                
                try {
                    // 找到这个类的所有成员
                    std::vector<ExportInfo> members;
                    for (const auto& item : export_items) {
                        if (item.parent_class == class_info.name || 
                            item.owner_class == class_info.name) {
                            members.push_back(item);
                        }
                    }
                    std::cout << "     找到 " << members.size() << " 个成员" << std::endl;
                    
                    // 调试输出
                    std::string debug_comment = "// DEBUG: Class " + class_info.name + " has " + 
                                              std::to_string(members.size()) + " members";
                    bindings_builder.AddLine(debug_comment);
                    for (const auto& member : members) {
                        std::string member_debug = "// DEBUG: Member - " + member.export_type + "::" + 
                                                 member.name + " (parent: " + member.parent_class + ")";
                        bindings_builder.AddLine(member_debug);
                    }
                    
                    std::cout << "     生成类绑定代码..." << std::endl;
                    bindings_builder.AddLine(GenerateClassBinding(class_info, members));
                    bindings_builder.AddEmptyLine();
                    result.total_bindings++;
                    std::cout << "   ✅ 类 " << class_info.name << " 绑定生成成功" << std::endl;
                    
                } catch (const std::exception& e) {
                    std::cout << "   ❌ 类 " << class_info.name << " 绑定生成失败: " << e.what() << std::endl;
                    throw;
                }
            }
        } else {
            std::cout << "   无类绑定需要处理" << std::endl;
        }
        
        // 生成静态类绑定
        if (grouped_exports.count("static_class") > 0) {
            bindings_builder.AddComment("Static class bindings");
            for (const auto& class_info : grouped_exports["static_class"]) {
                std::vector<ExportInfo> members;
                for (const auto& item : export_items) {
                    if (item.parent_class == class_info.name || 
                        item.owner_class == class_info.name) {
                        members.push_back(item);
                    }
                }
                
                // 调试输出
                std::string debug_comment = "// DEBUG: Static class " + class_info.name + " has " + 
                                          std::to_string(members.size()) + " members";
                bindings_builder.AddLine(debug_comment);
                for (const auto& member : members) {
                    std::string member_debug = "// DEBUG: Member - " + member.export_type + "::" + 
                                             member.name + " (parent: " + member.parent_class + ")";
                    bindings_builder.AddLine(member_debug);
                }
                
                bindings_builder.AddLine(GenerateStaticClassBinding(class_info, members));
                bindings_builder.AddEmptyLine();
                result.total_bindings++;
            }
        }
        
        // 生成抽象类绑定
        if (grouped_exports.count("abstract_class") > 0) {
            bindings_builder.AddComment("Abstract class bindings");
            for (const auto& class_info : grouped_exports["abstract_class"]) {
                std::vector<ExportInfo> members;
                for (const auto& item : export_items) {
                    if (item.parent_class == class_info.name || 
                        item.owner_class == class_info.name) {
                        members.push_back(item);
                    }
                }
                
                
                bindings_builder.AddLine(GenerateAbstractClassBinding(class_info, members));
                bindings_builder.AddEmptyLine();
                result.total_bindings++;
            }
        }
        
        // 生成单例绑定
        if (grouped_exports.count("singleton") > 0) {
            bindings_builder.AddComment("Singleton bindings");
            for (const auto& class_info : grouped_exports["singleton"]) {
                std::vector<ExportInfo> members;
                for (const auto& item : export_items) {
                    if (item.parent_class == class_info.name || 
                        item.owner_class == class_info.name) {
                        members.push_back(item);
                    }
                }
                bindings_builder.AddLine(GenerateSingletonBinding(class_info, members));
                bindings_builder.AddEmptyLine();
                result.total_bindings++;
            }
        }
        
        // 生成函数绑定
        if (grouped_exports.count("function") > 0) {
            bindings_builder.AddComment("Function bindings");
            
            // 按命名空间分组函数绑定
            std::unordered_map<std::string, std::vector<ExportInfo>> func_by_namespace;
            for (const auto& func_info : grouped_exports["function"]) {
                std::string ns = namespace_manager_.ResolveNamespace(func_info);
                func_by_namespace[ns].push_back(func_info);
            }
            
            // 为每个命名空间生成函数绑定
            for (const auto& [ns, funcs] : func_by_namespace) {
                if (funcs.size() > 1) {
                    std::string ns_comment = (ns == "global") ? "Global functions" : 
                                           "Functions in namespace " + ns;
                    bindings_builder.AddComment(ns_comment);
                }
                
                for (const auto& func_info : funcs) {
                    bindings_builder.AddLine(GenerateFunctionBinding(func_info));
                    result.total_bindings++;
                }
                
                if (funcs.size() > 1) {
                    bindings_builder.AddEmptyLine();
                }
            }
        }
        
        // 生成常量绑定
        if (grouped_exports.count("constant") > 0) {
            bindings_builder.AddComment("Constant bindings");
            
            // 按命名空间分组常量绑定
            std::unordered_map<std::string, std::vector<ExportInfo>> const_by_namespace;
            for (const auto& const_info : grouped_exports["constant"]) {
                std::string ns = namespace_manager_.ResolveNamespace(const_info);
                const_by_namespace[ns].push_back(const_info);
            }
            
            // 为每个命名空间生成常量绑定
            for (const auto& [ns, constants] : const_by_namespace) {
                if (constants.size() > 1) {
                    std::string ns_comment = (ns == "global") ? "Global constants" : 
                                           "Constants in namespace " + ns;
                    bindings_builder.AddComment(ns_comment);
                }
                
                for (const auto& const_info : constants) {
                    bindings_builder.AddLine(GenerateConstantBinding(const_info));
                    result.total_bindings++;
                }
                
                if (constants.size() > 1) {
                    bindings_builder.AddEmptyLine();
                }
            }
        }
        
        // 生成枚举绑定
        if (grouped_exports.count("enum") > 0) {
            bindings_builder.AddComment("Enum bindings");
            
            // 按命名空间分组枚举绑定
            std::unordered_map<std::string, std::vector<ExportInfo>> enum_by_namespace;
            for (const auto& enum_info : grouped_exports["enum"]) {
                std::string ns = namespace_manager_.ResolveNamespace(enum_info);
                enum_by_namespace[ns].push_back(enum_info);
            }
            
            // 为每个命名空间生成枚举绑定
            for (const auto& [ns, enums] : enum_by_namespace) {
                if (enums.size() > 1) {
                    std::string ns_comment = (ns == "global") ? "Global enums" : 
                                           "Enums in namespace " + ns;
                    bindings_builder.AddComment(ns_comment);
                }
                
                for (const auto& enum_info : enums) {
                    // 从 ExportInfo 提取枚举值
                    std::vector<std::string> enum_values;
                    if (enum_info.attributes.count("enum_values")) {
                        std::string values_str = enum_info.attributes.at("enum_values");
                        if (!values_str.empty()) {
                            std::stringstream ss(values_str);
                            std::string value;
                            while (std::getline(ss, value, ',')) {
                                enum_values.push_back(value);
                            }
                        }
                    }
                    bindings_builder.AddLine(GenerateEnumBinding(enum_info, enum_values));
                    result.total_bindings++;
                }
                
                if (enums.size() > 1) {
                    bindings_builder.AddEmptyLine();
                }
            }
        }
        
        // 生成 STL 绑定
        if (grouped_exports.count("stl") > 0) {
            bindings_builder.AddComment("STL container bindings");
            for (const auto& stl_info : grouped_exports["stl"]) {
                bindings_builder.AddLine(GenerateSTLBinding(stl_info));
                result.total_bindings++;
            }
        }
        
        bindings_code = bindings_builder.Build();
        
        // 5. 生成注册函数
        if (options_.generate_registration_function) {
            builder.AddLine(GenerateRegistrationFunction(module_name, bindings_code));
        } else {
            builder.AddLine(bindings_code);
        }
        
        result.generated_code = builder.Build();
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errors.push_back(std::string("Generation failed: ") + e.what());
    }
    
    return result;
}

std::string DirectBindingGenerator::GenerateClassBinding(const ExportInfo& class_info,
                                                       const std::vector<ExportInfo>& members) {
    // 检查参数数量，如果超过阈值则使用分批绑定
    const size_t MAX_PARAMETERS = 20; // Sol2的安全参数限制
    size_t param_count = CalculateBindingParameterCount(members, class_info);
    
    if (param_count > MAX_PARAMETERS) {
        // 使用分批绑定
        return GenerateBatchedClassBinding(class_info, members);
    }
    
    // 使用传统的单次绑定
    CodeBuilder builder(options_.indent_size);
    
    std::string namespace_var = namespace_manager_.GetNamespaceVariable(
        namespace_manager_.ResolveNamespace(class_info));
    
    std::string lua_class_name = class_info.lua_name.empty() ? class_info.name : class_info.lua_name;
    std::string qualified_name = GetQualifiedTypeName(class_info);
    
    // 开始类绑定 - 将类名放在同一行
    builder.AddIndentedLine(namespace_var + ".new_usertype<" + qualified_name + ">(\"" + lua_class_name + "\",");
    builder.IncreaseIndent();
    
    // 分类成员，并去除重复
    std::vector<ExportInfo> constructors, methods, static_methods, properties, operators;
    std::set<std::string> seen_signatures;
    
    for (const auto& member : members) {
        // 创建唯一签名来检测重复
        std::string signature = member.export_type + "::" + member.name + "::" + 
                               member.qualified_name + "::" + member.parent_class;
        
        if (seen_signatures.find(signature) != seen_signatures.end()) {
            continue; // 跳过重复项
        }
        seen_signatures.insert(signature);
        
        if (member.export_type == "constructor") {
            constructors.push_back(member);
        } else if (member.export_type == "method") {
            methods.push_back(member);
        } else if (member.export_type == "static_method") {
            static_methods.push_back(member);
        } else if (member.export_type == "property") {
            properties.push_back(member);
        } else if (member.export_type == "operator") {
            operators.push_back(member);
        }
    }
    
    // 收集所有绑定项
    std::vector<std::string> all_bindings;
    
    // 生成构造函数绑定 - 使用 Sol2 构造函数模板 (必须第一个)
    if (!constructors.empty()) {
        std::string ctor_binding = GenerateConstructorBindings(constructors);
        all_bindings.push_back(ctor_binding);
    } else {
        // 如果没有找到构造函数，生成默认的空构造函数
        all_bindings.push_back("sol::constructors<>()");
    }
    
    // 生成继承关系 (需要分成两个参数: sol::base_classes 和 sol::bases<...>())
    // 注意：只有当基类也被导出时才包含继承关系
    // 当前简化处理：暂时跳过继承关系声明，避免引用未导出的基类
    if (!class_info.base_classes.empty()) {
        // TODO: 添加基类导出状态检查逻辑
        // 现在暂时注释掉，避免引用未导出的Entity类
        /*
        all_bindings.push_back("sol::base_classes");
        
        std::string bases_spec = "sol::bases<";
        for (size_t i = 0; i < class_info.base_classes.size(); ++i) {
            if (i > 0) bases_spec += ", ";
            // Use fully qualified base class names
            std::string base_class = class_info.base_classes[i];
            if (base_class.find("::") == std::string::npos) {
                // Base class without namespace - need to fully qualify it
                std::string qualified_name = GetQualifiedTypeName(class_info);
                size_t last_scope = qualified_name.find_last_of("::");
                if (last_scope != std::string::npos) {
                    // Extract the namespace part
                    std::string base_namespace = qualified_name.substr(0, last_scope - 1);
                    base_class = base_namespace + "::" + base_class;
                }
            }
            bases_spec += base_class;
        }
        bases_spec += ">()";
        all_bindings.push_back(bases_spec);
        */
    }
    
    // 生成方法绑定
    for (const auto& method : methods) {
        std::string method_name = method.lua_name.empty() ? method.name : method.lua_name;
        
        // 跳过不能绑定的操作符
        if (method_name == "operator=" || method_name == "operator->" || method_name == "operator&") {
            continue;
        }
        
        std::string qualified_method_name = GetQualifiedTypeName(method);
        all_bindings.push_back("\"" + method_name + "\", &" + qualified_method_name);
    }
    
    // 生成静态方法绑定
    for (const auto& static_method : static_methods) {
        std::string static_name = static_method.lua_name.empty() ? static_method.name : static_method.lua_name;
        std::string qualified_static_name = GetQualifiedTypeName(static_method);
        all_bindings.push_back("\"" + static_name + "\", &" + qualified_static_name);
    }
    
    // 生成属性绑定
    for (const auto& prop : properties) {
        std::string prop_name = prop.lua_name.empty() ? prop.name : prop.lua_name;
        std::string qualified_prop_name = GetQualifiedTypeName(prop);
        
        if (prop.property_access == "readonly") {
            all_bindings.push_back("\"" + prop_name + "\", sol::readonly_property(&" + qualified_prop_name + ")");
        } else if (prop.property_access == "readwrite") {
            // 查找 setter 方法
            std::string setter_name;
            if (prop.attributes.count("setter")) {
                setter_name = prop.attributes.at("setter");
            } else {
                // 默认 setter 名称：将 getXxx 替换为 setXxx
                std::string getter_name = prop.name;
                if (getter_name.find("get") == 0 && getter_name.length() > 3) {
                    setter_name = "set" + getter_name.substr(3);
                } else {
                    setter_name = "set" + getter_name;
                }
            }
            
            // 构造 setter 的完全限定名
            std::string setter_qualified_name;
            if (!prop.namespace_name.empty()) {
                setter_qualified_name = prop.namespace_name + "::";
            }
            if (!prop.parent_class.empty()) {
                setter_qualified_name += prop.parent_class + "::";
            }
            setter_qualified_name += setter_name;
            
            all_bindings.push_back("\"" + prop_name + "\", sol::property(&" + qualified_prop_name + ", &" + setter_qualified_name + ")");
        } else {
            // 默认为只读
            all_bindings.push_back("\"" + prop_name + "\", sol::readonly_property(&" + qualified_prop_name + ")");
        }
    }
    
    // 生成操作符绑定
    for (const auto& op : operators) {
        std::string op_binding = GenerateOperatorBinding(op);
        if (!op_binding.empty()) {
            all_bindings.push_back(op_binding);
        }
    }
    
    // 输出所有绑定项，除了最后一项外都加逗号
    if (all_bindings.empty()) {
        // 如果没有绑定项，生成一个空的构造函数
        builder.AddIndentedLine("sol::constructors<>()");
    } else {
        for (size_t i = 0; i < all_bindings.size(); ++i) {
            if (i == all_bindings.size() - 1) {
                // 最后一项不加逗号
                builder.AddIndentedLine(all_bindings[i]);
            } else {
                // 其他项加逗号
                builder.AddIndentedLine(all_bindings[i] + ",");
            }
        }
    }
    
    builder.DecreaseIndent();
    builder.AddIndentedLine(");");
    
    return builder.Build();
}

std::string DirectBindingGenerator::GenerateAbstractClassBinding(const ExportInfo& class_info,
                                                               const std::vector<ExportInfo>& members) {
    // 为抽象类生成绑定，不包含构造函数
    CodeBuilder builder(options_.indent_size);
    
    std::string namespace_var = namespace_manager_.GetNamespaceVariable(
        namespace_manager_.ResolveNamespace(class_info));
    
    std::string lua_class_name = class_info.lua_name.empty() ? class_info.name : class_info.lua_name;
    std::string qualified_name = GetQualifiedTypeName(class_info);
    
    // 开始类绑定 - 将类名放在同一行
    builder.AddIndentedLine("auto " + lua_class_name + "_type = " + namespace_var + ".new_usertype<" + qualified_name + ">(\"" + lua_class_name + "\",");
    builder.IncreaseIndent();
    
    // 分类成员，并去除重复（排除构造函数）
    std::vector<ExportInfo> methods, static_methods, properties, operators;
    std::set<std::string> seen_signatures;
    
    for (const auto& member : members) {
        // 跳过构造函数 - 抽象类不能实例化
        if (member.export_type == "constructor") {
            continue;
        }
        
        // 创建唯一签名来检测重复
        std::string signature = member.export_type + "::" + member.name + "::" + 
                               member.qualified_name + "::" + member.parent_class;
        
        if (seen_signatures.find(signature) != seen_signatures.end()) {
            continue; // 跳过重复项
        }
        seen_signatures.insert(signature);
        
        if (member.export_type == "method") {
            methods.push_back(member);
        } else if (member.export_type == "static_method") {
            static_methods.push_back(member);
        } else if (member.export_type == "property") {
            properties.push_back(member);
        } else if (member.export_type == "operator") {
            operators.push_back(member);
        }
    }
    
    // 收集所有绑定项
    std::vector<std::string> all_bindings;
    
    // 抽象类没有构造函数，但需要一个空的构造函数参数来满足Sol2语法
    all_bindings.push_back("sol::constructors<>()");
    
    // 生成继承关系 - 暂时跳过，避免引用未导出的基类Entity
    // TODO: 添加基类导出状态检查逻辑
    /*
    if (!class_info.base_classes.empty()) {
        all_bindings.push_back("sol::base_classes");
        
        std::string bases_spec = "sol::bases<";
        for (size_t i = 0; i < class_info.base_classes.size(); ++i) {
            if (i > 0) bases_spec += ", ";
            std::string base_class = class_info.base_classes[i];
            if (base_class.find("::") == std::string::npos) {
                std::string qualified_name = GetQualifiedTypeName(class_info);
                size_t last_scope = qualified_name.find_last_of("::");
                if (last_scope != std::string::npos) {
                    std::string base_namespace = qualified_name.substr(0, last_scope - 1);
                    base_class = base_namespace + "::" + base_class;
                }
            }
            bases_spec += base_class;
        }
        bases_spec += ">()";
        all_bindings.push_back(bases_spec);
    }
    */
    
    // 将所有绑定合并为一行或多行
    if (all_bindings.empty()) {
        builder.AddIndentedLine("sol::constructors<>()");
    } else {
        for (size_t i = 0; i < all_bindings.size(); ++i) {
            if (i == all_bindings.size() - 1) {
                builder.AddIndentedLine(all_bindings[i]);
            } else {
                builder.AddIndentedLine(all_bindings[i] + ",");
            }
        }
    }
    
    builder.DecreaseIndent();
    builder.AddIndentedLine(");");
    builder.AddEmptyLine();
    
    // 单独添加方法绑定
    for (const auto& method : methods) {
        std::string lua_name = method.lua_name.empty() ? method.name : method.lua_name;
        std::string qualified_method = GetQualifiedTypeName(method);
        builder.AddIndentedLine(lua_class_name + "_type[\"" + lua_name + "\"] = &" + qualified_method + ";");
    }
    
    // 添加静态方法绑定
    for (const auto& static_method : static_methods) {
        std::string lua_name = static_method.lua_name.empty() ? static_method.name : static_method.lua_name;
        std::string qualified_method = GetQualifiedTypeName(static_method);
        builder.AddIndentedLine(lua_class_name + "_type[\"" + lua_name + "\"] = &" + qualified_method + ";");
    }
    
    // 添加属性绑定
    for (const auto& prop : properties) {
        std::string lua_name = prop.lua_name.empty() ? prop.name : prop.lua_name;
        std::string qualified_prop = GetQualifiedTypeName(prop);
        
        if (prop.property_access == "readonly") {
            builder.AddIndentedLine(lua_class_name + "_type[\"" + lua_name + "\"] = sol::readonly_property(&" + qualified_prop + ");");
        } else if (prop.property_access == "readwrite") {
            // 查找setter方法
            std::string setter_name;
            if (prop.attributes.count("setter")) {
                setter_name = prop.attributes.at("setter");
            } else {
                std::string getter_name = prop.name;
                if (getter_name.find("get") == 0 && getter_name.length() > 3) {
                    setter_name = "set" + getter_name.substr(3);
                } else {
                    setter_name = "set" + getter_name;
                }
            }
            
            std::string setter_qualified = prop.owner_class + "::" + setter_name;
            builder.AddIndentedLine(lua_class_name + "_type[\"" + lua_name + "\"] = sol::property(&" + qualified_prop + ", &" + setter_qualified + ");");
        } else {
            builder.AddIndentedLine(lua_class_name + "_type[\"" + lua_name + "\"] = sol::property(&" + qualified_prop + ");");
        }
    }
    
    // 添加操作符绑定
    for (const auto& op : operators) {
        std::string op_binding = GenerateOperatorBinding(op);
        if (!op_binding.empty()) {
            builder.AddIndentedLine(lua_class_name + "_type[" + op_binding + ";");
        }
    }
    
    return builder.Build();
}

size_t DirectBindingGenerator::CalculateBindingParameterCount(const std::vector<ExportInfo>& members,
                                                             const ExportInfo& class_info) {
    size_t count = 0;
    
    // 计算基础参数：构造函数（1个）
    count += 1;
    
    // 计算继承关系参数：暂时跳过，避免引用未导出的基类Entity
    // TODO: 添加基类导出状态检查逻辑
    /*
    if (!class_info.base_classes.empty()) {
        count += 2;
    }
    */
    
    // 计算成员参数：每个成员需要2个参数（名称 + 指针）
    for (const auto& member : members) {
        if (member.export_type == "constructor") {
            // 构造函数已经包含在基础参数中
            continue;
        } else if (member.export_type == "method" || 
                   member.export_type == "static_method" ||
                   member.export_type == "property") {
            count += 2; // 名称 + 指针/property wrapper
        } else if (member.export_type == "operator") {
            count += 2; // meta_function + 指针
        }
    }
    
    return count;
}

std::string DirectBindingGenerator::GenerateBatchedClassBinding(const ExportInfo& class_info,
                                                               const std::vector<ExportInfo>& members) {
    CodeBuilder builder(options_.indent_size);
    
    std::string namespace_var = namespace_manager_.GetNamespaceVariable(
        namespace_manager_.ResolveNamespace(class_info));
    
    std::string lua_class_name = class_info.lua_name.empty() ? class_info.name : class_info.lua_name;
    std::string qualified_name = GetQualifiedTypeName(class_info);
    
    // 分类成员，并去除重复
    std::vector<ExportInfo> constructors, methods, static_methods, properties, operators;
    std::set<std::string> seen_signatures;
    
    for (const auto& member : members) {
        // 创建唯一签名来检测重复
        std::string signature = member.export_type + "::" + member.name + "::" + 
                               member.qualified_name + "::" + member.parent_class;
        
        if (seen_signatures.find(signature) != seen_signatures.end()) {
            continue; // 跳过重复项
        }
        seen_signatures.insert(signature);
        
        if (member.export_type == "constructor") {
            constructors.push_back(member);
        } else if (member.export_type == "method") {
            methods.push_back(member);
        } else if (member.export_type == "static_method") {
            static_methods.push_back(member);
        } else if (member.export_type == "property") {
            properties.push_back(member);
        } else if (member.export_type == "operator") {
            operators.push_back(member);
        }
    }
    
    // 第一步：创建基础类型（构造函数 + 继承关系）
    builder.AddIndentedLine("auto " + lua_class_name + "_type = " + namespace_var + ".new_usertype<" + qualified_name + ">(\"" + lua_class_name + "\",");
    builder.IncreaseIndent();
    
    std::vector<std::string> basic_bindings;
    
    // 生成构造函数绑定
    if (!constructors.empty()) {
        std::string ctor_binding = GenerateConstructorBindings(constructors);
        basic_bindings.push_back(ctor_binding);
    } else {
        basic_bindings.push_back("sol::constructors<>()");
    }
    
    // 生成继承关系 - 暂时跳过，避免引用未导出的基类Entity
    // TODO: 添加基类导出状态检查逻辑
    /*
    if (!class_info.base_classes.empty()) {
        basic_bindings.push_back("sol::base_classes");
        
        std::string bases_spec = "sol::bases<";
        for (size_t i = 0; i < class_info.base_classes.size(); ++i) {
            if (i > 0) bases_spec += ", ";
            // Use fully qualified base class names
            std::string base_class = class_info.base_classes[i];
            if (base_class.find("::") == std::string::npos) {
                // Base class without namespace - need to fully qualify it
                std::string qualified_name_local = GetQualifiedTypeName(class_info);
                size_t last_scope = qualified_name_local.find_last_of("::");
                if (last_scope != std::string::npos) {
                    // Extract the namespace part
                    std::string base_namespace = qualified_name_local.substr(0, last_scope - 1);
                    base_class = base_namespace + "::" + base_class;
                }
            }
            bases_spec += base_class;
        }
        bases_spec += ">()";
        basic_bindings.push_back(bases_spec);
    }
    */
    
    // 输出基础绑定
    for (size_t i = 0; i < basic_bindings.size(); ++i) {
        if (i == basic_bindings.size() - 1) {
            builder.AddIndentedLine(basic_bindings[i]);
        } else {
            builder.AddIndentedLine(basic_bindings[i] + ",");
        }
    }
    
    builder.DecreaseIndent();
    builder.AddIndentedLine(");");
    builder.AddEmptyLine();
    
    // 第二步：分批添加成员方法
    for (const auto& method : methods) {
        std::string method_name = method.lua_name.empty() ? method.name : method.lua_name;
        
        // 跳过不能绑定的操作符
        if (method_name == "operator=" || method_name == "operator->" || method_name == "operator&") {
            continue;
        }
        
        std::string qualified_method_name = GetQualifiedTypeName(method);
        builder.AddIndentedLine(lua_class_name + "_type[\"" + method_name + "\"] = &" + qualified_method_name + ";");
    }
    
    // 添加静态方法
    for (const auto& static_method : static_methods) {
        std::string static_name = static_method.lua_name.empty() ? static_method.name : static_method.lua_name;
        std::string qualified_static_name = GetQualifiedTypeName(static_method);
        builder.AddIndentedLine(lua_class_name + "_type[\"" + static_name + "\"] = &" + qualified_static_name + ";");
    }
    
    // 添加属性
    for (const auto& prop : properties) {
        std::string prop_name = prop.lua_name.empty() ? prop.name : prop.lua_name;
        std::string qualified_prop_name = GetQualifiedTypeName(prop);
        
        if (prop.property_access == "readonly") {
            builder.AddIndentedLine(lua_class_name + "_type[\"" + prop_name + "\"] = sol::readonly_property(&" + qualified_prop_name + ");");
        } else {
            builder.AddIndentedLine(lua_class_name + "_type[\"" + prop_name + "\"] = sol::property(&" + qualified_prop_name + ");");
        }
    }
    
    // 添加操作符重载（使用sol::meta_function）
    for (const auto& op : operators) {
        std::string op_binding = GenerateOperatorBinding(op);
        if (!op_binding.empty()) {
            // 解析操作符绑定（格式：meta_function, &qualified_name）
            size_t comma_pos = op_binding.find(",");
            if (comma_pos != std::string::npos) {
                std::string meta_func = op_binding.substr(0, comma_pos);
                std::string op_ptr = op_binding.substr(comma_pos + 2); // +2 跳过", "
                builder.AddIndentedLine(lua_class_name + "_type[" + meta_func + "] = " + op_ptr + ";");
            }
        }
    }
    
    return builder.Build();
}

std::string DirectBindingGenerator::GenerateFunctionBinding(const ExportInfo& function_info) {
    std::string namespace_var = namespace_manager_.GetNamespaceVariable(
        namespace_manager_.ResolveNamespace(function_info));
    
    std::string lua_name = function_info.lua_name.empty() ? function_info.name : function_info.lua_name;
    std::string qualified_name = GetQualifiedTypeName(function_info);
    
    if (namespace_var == "lua") {
        return "lua[\"" + lua_name + "\"] = &" + qualified_name + ";";
    } else {
        return namespace_var + "[\"" + lua_name + "\"] = &" + qualified_name + ";";
    }
}

std::string DirectBindingGenerator::GenerateEnumBinding(const ExportInfo& enum_info,
                                                       const std::vector<std::string>& enum_values) {
    CodeBuilder builder(options_.indent_size);
    
    std::string namespace_var = namespace_manager_.GetNamespaceVariable(
        namespace_manager_.ResolveNamespace(enum_info));
    
    std::string lua_name = enum_info.lua_name.empty() ? enum_info.name : enum_info.lua_name;
    std::string qualified_name = GetQualifiedTypeName(enum_info);
    
    if (enum_values.empty()) {
        // 如果没有枚举值，生成简化版本
        builder.AddIndentedLine(namespace_var + ".new_enum(\"" + lua_name + "\");");
    } else {
        // 多行格式，每个枚举值一行
        builder.AddIndentedLine(namespace_var + ".new_enum(\"" + lua_name + "\"");
        builder.IncreaseIndent();
        
        for (size_t i = 0; i < enum_values.size(); ++i) {
            const auto& value = enum_values[i];
            std::string prefix = (i == 0 ? ", " : ", ");
            builder.AddIndentedLine(prefix + "\"" + value + "\", " + 
                                   qualified_name + "::" + value);
        }
        
        builder.DecreaseIndent();
        builder.AddIndentedLine(");");
    }
    
    return builder.Build();
}

std::string DirectBindingGenerator::GenerateSTLBinding(const ExportInfo& stl_info) {
    auto type_info = AnalyzeSTLType(stl_info.type_name);
    
    // 使用容器的alias作为Lua类型名称，如果有的话
    if (stl_info.attributes.count("alias")) {
        type_info.lua_type_name = stl_info.attributes.at("alias");
    } else {
        // 生成基于类型的唯一名称
        type_info.lua_type_name = stl_info.name;
    }
    
    switch (type_info.container_type) {
        case STLTypeInfo::VECTOR:
            return GenerateVectorBinding(type_info);
        case STLTypeInfo::MAP:
            return GenerateMapBinding(type_info);
        case STLTypeInfo::UNORDERED_MAP:
            return GenerateUnorderedMapBinding(type_info);
        case STLTypeInfo::SET:
            return GenerateSetBinding(type_info);
        case STLTypeInfo::LIST:
            return GenerateListBinding(type_info);
        default:
            return "// Unsupported STL container: " + stl_info.type_name;
    }
}

std::string DirectBindingGenerator::GenerateCallbackBinding(const ExportInfo& callback_info) {
    // 回调函数通常作为类成员变量处理
    std::string lua_name = callback_info.lua_name.empty() ? callback_info.name : callback_info.lua_name;
    return "\"" + lua_name + "\", &" + GetQualifiedTypeName(callback_info);
}

std::string DirectBindingGenerator::GenerateStaticClassBinding(const ExportInfo& class_info,
                                                              const std::vector<ExportInfo>& members) {
    CodeBuilder builder(options_.indent_size);
    
    std::string namespace_var = namespace_manager_.GetNamespaceVariable(
        namespace_manager_.ResolveNamespace(class_info));
    
    std::string lua_class_name = class_info.lua_name.empty() ? class_info.name : class_info.lua_name;
    
    // 为静态类创建一个table，而不是usertype
    builder.AddIndentedLine("auto " + lua_class_name + "_table = " + namespace_var + "[\"" + lua_class_name + "\"].get_or_create<sol::table>();");
    
    // 只绑定静态方法
    for (const auto& member : members) {
        if (member.export_type == "static_method") {
            std::string method_name = member.lua_name.empty() ? member.name : member.lua_name;
            std::string qualified_name = GetQualifiedTypeName(member);
            builder.AddIndentedLine(lua_class_name + "_table[\"" + method_name + "\"] = &" + qualified_name + ";");
        }
    }
    
    return builder.Build();
}

std::string DirectBindingGenerator::GenerateSingletonBinding(const ExportInfo& class_info,
                                                            const std::vector<ExportInfo>& members) {
    CodeBuilder builder(options_.indent_size);
    
    std::string namespace_var = namespace_manager_.GetNamespaceVariable(
        namespace_manager_.ResolveNamespace(class_info));
    
    std::string lua_class_name = class_info.lua_name.empty() ? class_info.name : class_info.lua_name;
    std::string qualified_name = GetQualifiedTypeName(class_info);
    
    // 为单例类创建usertype，但添加getInstance方法
    builder.AddIndentedLine(namespace_var + ".new_usertype<" + qualified_name + ">(\"" + lua_class_name + "\",");
    builder.IncreaseIndent();
    
    std::vector<std::string> all_bindings;
    
    // 添加getInstance静态方法
    all_bindings.push_back("\"getInstance\", &" + qualified_name + "::getInstance");
    
    // 添加其他方法
    for (const auto& member : members) {
        if (member.export_type == "method") {
            std::string method_name = member.lua_name.empty() ? member.name : member.lua_name;
            std::string qualified_method_name = GetQualifiedTypeName(member);
            all_bindings.push_back("\"" + method_name + "\", &" + qualified_method_name);
        }
    }
    
    // 输出所有绑定项
    for (size_t i = 0; i < all_bindings.size(); ++i) {
        if (i == all_bindings.size() - 1) {
            builder.AddIndentedLine(all_bindings[i]);
        } else {
            builder.AddIndentedLine(all_bindings[i] + ",");
        }
    }
    
    builder.DecreaseIndent();
    builder.AddIndentedLine(");");
    
    return builder.Build();
}

std::string DirectBindingGenerator::GenerateConstantBinding(const ExportInfo& constant_info) {
    // 过滤掉无效的常量
    if (constant_info.name.find("__lua_module_marker_") == 0) {
        return "// Skipped module marker constant: " + constant_info.name;
    }
    
    std::string namespace_var = namespace_manager_.GetNamespaceVariable(
        namespace_manager_.ResolveNamespace(constant_info));
    
    std::string lua_name = constant_info.lua_name.empty() ? constant_info.name : constant_info.lua_name;
    std::string qualified_name = GetQualifiedTypeName(constant_info);
    
    // 为常量生成绑定，使用正确的命名空间
    if (namespace_var == "lua") {
        return "lua[\"" + lua_name + "\"] = " + qualified_name + ";";
    } else {
        return namespace_var + "[\"" + lua_name + "\"] = " + qualified_name + ";";
    }
}

std::string DirectBindingGenerator::GenerateOperatorBinding(const ExportInfo& operator_info) {
    std::string op_name = operator_info.name;
    std::string qualified_name = GetQualifiedTypeName(operator_info);
    
    // 映射操作符名称到Sol2的meta_function
    std::string sol_meta_func;
    if (op_name == "operator+") {
        sol_meta_func = "sol::meta_function::addition";
    } else if (op_name == "operator-") {
        sol_meta_func = "sol::meta_function::subtraction";
    } else if (op_name == "operator*") {
        sol_meta_func = "sol::meta_function::multiplication";
    } else if (op_name == "operator/") {
        sol_meta_func = "sol::meta_function::division";
    } else if (op_name == "operator==") {
        sol_meta_func = "sol::meta_function::equal_to";
    } else if (op_name == "operator!=") {
        // Sol2 doesn't have not_equal meta function, skip it
        return "";
    } else if (op_name == "operator<") {
        sol_meta_func = "sol::meta_function::less_than";
    } else if (op_name == "operator<=") {
        sol_meta_func = "sol::meta_function::less_than_or_equal_to";
    } else if (op_name == "operator>") {
        sol_meta_func = "sol::meta_function::greater_than";
    } else if (op_name == "operator>=") {
        sol_meta_func = "sol::meta_function::greater_than_or_equal_to";
    } else if (op_name == "operator[]") {
        sol_meta_func = "sol::meta_function::index";
    } else if (op_name == "operator()") {
        sol_meta_func = "sol::meta_function::call";
    } else {
        // 不支持的操作符，返回空字符串
        return "";
    }
    
    return sol_meta_func + ", &" + qualified_name;
}

// 私有辅助方法实现
std::string DirectBindingGenerator::GenerateFileHeader(const std::string& module_name) {
    CodeBuilder builder;
    
    builder.AddBlockComment({
        "@file " + module_name + "_bindings.cpp",
        "@brief Auto-generated Lua bindings for " + module_name + " module",
        "",
        "This file is automatically generated by lua_binding_generator.",
        "Do not modify this file manually."
    });
    
    return builder.Build();
}

std::string DirectBindingGenerator::GenerateIncludes(const std::vector<ExportInfo>& export_items) {
    CodeBuilder builder;
    
    // 标准包含
    builder.AddLine("#include <sol/sol.hpp>");
    
    // 收集需要的头文件
    std::set<std::string> required_includes;
    for (const auto& item : export_items) {
        if (!item.file_path.empty()) {
            // 提取文件名，去除路径
            std::string filename = item.file_path;
            size_t slash_pos = filename.find_last_of("/\\");
            if (slash_pos != std::string::npos) {
                filename = filename.substr(slash_pos + 1);
            }
            required_includes.insert(filename);
        }
    }
    
    // 添加用户头文件
    for (const auto& include : required_includes) {
        builder.AddLine("#include \"" + include + "\"");
    }
    
    return builder.Build();
}

std::string DirectBindingGenerator::GenerateNamespaceDeclarations() {
    CodeBuilder builder;
    
    auto namespaces = namespace_manager_.GetRequiredNamespaces();
    for (const auto& ns : namespaces) {
        if (ns != "global") {
            std::string var_name = namespace_manager_.GetNamespaceVariable(ns);
            
            // 处理命名空间，支持 :: 和 . 分隔符
            std::string lua_path = "lua";
            std::string processed_ns = ns;
            
            // 替换 :: 为 .
            size_t pos = 0;
            while ((pos = processed_ns.find("::", pos)) != std::string::npos) {
                processed_ns.replace(pos, 2, ".");
                pos += 1;
            }
            
            std::istringstream iss(processed_ns);
            std::string part;
            while (std::getline(iss, part, '.')) {
                if (!part.empty()) {
                    lua_path += "[\"" + part + "\"]";
                }
            }
            
            builder.AddIndentedLine("auto " + var_name + " = " + lua_path + 
                                   ".get_or_create<sol::table>();");
        }
    }
    
    return builder.Build();
}

std::string DirectBindingGenerator::GenerateRegistrationFunction(const std::string& module_name,
                                                                const std::string& bindings_code) {
    CodeBuilder builder;
    
    builder.AddLine("void register_" + module_name + "_bindings(sol::state& lua) {");
    builder.IncreaseIndent();
    builder.AddLine(bindings_code);
    builder.DecreaseIndent();
    builder.AddLine("}");
    
    return builder.Build();
}

std::string DirectBindingGenerator::GenerateConstructorBindings(const std::vector<ExportInfo>& constructors) {
    if (constructors.empty()) {
        return "sol::constructors<>()";
    }
    
    std::string result = "sol::constructors<";
    std::set<std::string> unique_signatures; // 去重构造函数签名
    bool first = true;
    
    for (const auto& ctor : constructors) {
        // 构建构造函数参数签名 - Sol2 需要完全限定的类名加参数类型
        std::string signature;
        
        // 获取完全限定的类名
        std::string qualified_class_name = ctor.namespace_name.empty() ? 
            ctor.parent_class : 
            ctor.namespace_name + "::" + ctor.parent_class;
        
        if (ctor.parameter_types.empty()) {
            // 默认构造函数
            signature = qualified_class_name + "()";
        } else {
            // 有参数的构造函数
            signature = qualified_class_name + "(";
            for (size_t j = 0; j < ctor.parameter_types.size(); ++j) {
                if (j > 0) signature += ", ";
                // 清理参数类型，确保格式正确
                std::string param_type = ctor.parameter_types[j];
                // 移除多余的空格
                param_type = std::regex_replace(param_type, std::regex("\\s+"), " ");
                // 移除前后空格
                param_type = std::regex_replace(param_type, std::regex("^\\s+|\\s+$"), "");
                signature += param_type;
            }
            signature += ")";
        }
        
        // 检查是否已存在相同签名
        if (unique_signatures.find(signature) != unique_signatures.end()) {
            continue;
        }
        unique_signatures.insert(signature);
        
        if (!first) {
            result += ", ";
        }
        result += signature;
        first = false;
    }
    
    result += ">()";
    
    return result;
}

std::vector<std::string> DirectBindingGenerator::SplitConstructorBindings(const std::vector<ExportInfo>& constructors) {
    std::vector<std::string> lines;
    
    if (constructors.empty()) {
        lines.push_back("sol::constructors<>()");
        return lines;
    }
    
    // 为每个构造函数创建一个绑定
    std::set<std::string> seen_ctors;
    for (const auto& ctor : constructors) {
        std::string ctor_name = ctor.lua_name.empty() ? ctor.name : ctor.lua_name;
        std::string qualified_name = GetQualifiedTypeName(ctor);
        
        // 避免重复的构造函数绑定
        std::string signature = ctor_name + "::" + qualified_name;
        if (seen_ctors.find(signature) != seen_ctors.end()) {
            continue;
        }
        seen_ctors.insert(signature);
        
        lines.push_back("\"" + ctor_name + "\", &" + qualified_name);
    }
    
    return lines;
}

std::string DirectBindingGenerator::GenerateMethodBindings(const std::vector<ExportInfo>& methods) {
    std::string result;
    
    for (size_t i = 0; i < methods.size(); ++i) {
        const auto& method = methods[i];
        std::string lua_name = method.lua_name.empty() ? method.name : method.lua_name;
        
        if (i > 0) result += ", ";
        result += "\"" + lua_name + "\", &" + GetQualifiedTypeName(method);
    }
    
    return result;
}

std::vector<std::string> DirectBindingGenerator::SplitMethodBindings(const std::vector<ExportInfo>& methods) {
    std::vector<std::string> lines;
    std::set<std::string> seen_methods;
    
    for (const auto& method : methods) {
        std::string lua_name = method.lua_name.empty() ? method.name : method.lua_name;
        std::string qualified_name = GetQualifiedTypeName(method);
        
        // 避免重复的方法绑定
        std::string signature = lua_name + "::" + qualified_name;
        if (seen_methods.find(signature) != seen_methods.end()) {
            continue;
        }
        seen_methods.insert(signature);
        
        lines.push_back("\"" + lua_name + "\", &" + qualified_name);
    }
    
    return lines;
}

std::string DirectBindingGenerator::GeneratePropertyBindings(const std::vector<ExportInfo>& properties) {
    std::string result;
    
    for (size_t i = 0; i < properties.size(); ++i) {
        const auto& prop = properties[i];
        std::string lua_name = prop.lua_name.empty() ? prop.name : prop.lua_name;
        
        if (i > 0) result += ", ";
        
        if (prop.property_access == "readonly") {
            result += "\"" + lua_name + "\", sol::readonly_property(&" + GetQualifiedTypeName(prop) + ")";
        } else if (prop.property_access == "readwrite") {
            // 查找setter方法
            std::string setter_name;
            if (prop.attributes.count("setter")) {
                setter_name = prop.attributes.at("setter");
            } else {
                // 默认setter名称：将getXxx替换为setXxx
                std::string getter_name = prop.name;
                if (getter_name.find("get") == 0 && getter_name.length() > 3) {
                    setter_name = "set" + getter_name.substr(3);
                } else {
                    setter_name = "set" + getter_name;
                }
            }
            
            std::string setter_qualified = prop.owner_class + "::" + setter_name;
            result += "\"" + lua_name + "\", sol::property(&" + GetQualifiedTypeName(prop) + ", &" + setter_qualified + ")";
        } else {
            // 默认为只读属性
            result += "\"" + lua_name + "\", sol::property(&" + GetQualifiedTypeName(prop) + ")";
        }
    }
    
    return result;
}

std::vector<std::string> DirectBindingGenerator::SplitPropertyBindings(const std::vector<ExportInfo>& properties) {
    std::vector<std::string> lines;
    std::set<std::string> seen_properties;
    
    for (const auto& prop : properties) {
        std::string lua_name = prop.lua_name.empty() ? prop.name : prop.lua_name;
        std::string qualified_name = GetQualifiedTypeName(prop);
        
        // 避免重复的属性绑定
        std::string signature = lua_name + "::" + qualified_name;
        if (seen_properties.find(signature) != seen_properties.end()) {
            continue;
        }
        seen_properties.insert(signature);
        
        std::string binding;
        if (prop.property_access == "readonly") {
            binding = "\"" + lua_name + "\", sol::readonly_property(&" + qualified_name + ")";
        } else if (prop.property_access == "readwrite") {
            // 查找setter方法
            std::string setter_name;
            if (prop.attributes.count("setter")) {
                setter_name = prop.attributes.at("setter");
            } else {
                // 默认setter名称：将getXxx替换为setXxx
                std::string getter_name = prop.name;
                if (getter_name.find("get") == 0 && getter_name.length() > 3) {
                    setter_name = "set" + getter_name.substr(3);
                } else {
                    setter_name = "set" + getter_name;
                }
            }
            
            std::string setter_qualified = prop.owner_class + "::" + setter_name;
            binding = "\"" + lua_name + "\", sol::property(&" + qualified_name + ", &" + setter_qualified + ")";
        } else {
            // 默认为只读属性
            binding = "\"" + lua_name + "\", sol::property(&" + qualified_name + ")";
        }
        
        lines.push_back(binding);
    }
    
    return lines;
}

std::string DirectBindingGenerator::GenerateStaticMethodBindings(const std::vector<ExportInfo>& static_methods) {
    std::string result;
    
    for (size_t i = 0; i < static_methods.size(); ++i) {
        const auto& method = static_methods[i];
        std::string lua_name = method.lua_name.empty() ? method.name : method.lua_name;
        
        if (i > 0) result += ", ";
        result += "\"" + lua_name + "\", &" + GetQualifiedTypeName(method);
    }
    
    return result;
}

std::vector<std::string> DirectBindingGenerator::SplitStaticMethodBindings(const std::vector<ExportInfo>& static_methods) {
    std::vector<std::string> lines;
    std::set<std::string> seen_static_methods;
    
    for (const auto& method : static_methods) {
        std::string lua_name = method.lua_name.empty() ? method.name : method.lua_name;
        std::string qualified_name = GetQualifiedTypeName(method);
        
        // 避免重复的静态方法绑定
        std::string signature = lua_name + "::" + qualified_name;
        if (seen_static_methods.find(signature) != seen_static_methods.end()) {
            continue;
        }
        seen_static_methods.insert(signature);
        
        lines.push_back("\"" + lua_name + "\", &" + qualified_name);
    }
    
    return lines;
}

std::string DirectBindingGenerator::GenerateOperatorBindings(const std::vector<ExportInfo>& operators) {
    // TODO: 实现操作符绑定
    return "";
}

std::vector<std::string> DirectBindingGenerator::SplitOperatorBindings(const std::vector<ExportInfo>& operators) {
    std::vector<std::string> lines;
    std::set<std::string> seen_operators;
    
    for (const auto& op : operators) {
        std::string lua_name = op.lua_name.empty() ? op.name : op.lua_name;
        std::string qualified_name = GetQualifiedTypeName(op);
        
        // 避免重复的操作符绑定
        std::string signature = lua_name + "::" + qualified_name;
        if (seen_operators.find(signature) != seen_operators.end()) {
            continue;
        }
        seen_operators.insert(signature);
        
        lines.push_back("\"" + lua_name + "\", &" + qualified_name);
    }
    
    return lines;
}

std::string DirectBindingGenerator::GenerateInheritanceCode(const ExportInfo& class_info) {
    if (class_info.base_classes.empty()) {
        return "";
    }
    
    // Return as one binding pair to satisfy Sol2's even argument requirement
    std::string result = "sol::base_classes, sol::bases<";
    for (size_t i = 0; i < class_info.base_classes.size(); ++i) {
        if (i > 0) result += ", ";
        // Use fully qualified base class names
        std::string base_class = class_info.base_classes[i];
        if (base_class.find("::") == std::string::npos) {
            // Base class without namespace - need to fully qualify it
            std::string qualified_name = GetQualifiedTypeName(class_info);
            size_t last_scope = qualified_name.find_last_of("::");
            if (last_scope != std::string::npos) {
                // Extract the namespace part
                std::string base_namespace = qualified_name.substr(0, last_scope - 1);
                base_class = base_namespace + "::" + base_class;
            }
        }
        result += base_class;
    }
    result += ">()";
    
    return result;
}

DirectBindingGenerator::STLTypeInfo DirectBindingGenerator::AnalyzeSTLType(const std::string& type_name) {
    STLTypeInfo info;
    info.full_type_name = type_name;
    info.container_type = STLTypeInfo::UNKNOWN;
    
    // 简单的类型分析
    if (type_name.find("std::vector") == 0) {
        info.container_type = STLTypeInfo::VECTOR;
        info.lua_type_name = "vector"; // TODO: 添加模板参数
    } else if (type_name.find("std::map") == 0) {
        info.container_type = STLTypeInfo::MAP;
        info.lua_type_name = "map";
    } else if (type_name.find("std::unordered_map") == 0) {
        info.container_type = STLTypeInfo::UNORDERED_MAP;
        info.lua_type_name = "unordered_map";
    } else if (type_name.find("std::set") == 0) {
        info.container_type = STLTypeInfo::SET;
        info.lua_type_name = "set";
    } else if (type_name.find("std::list") == 0) {
        info.container_type = STLTypeInfo::LIST;
        info.lua_type_name = "list";
    }
    
    // TODO: 解析模板参数
    
    return info;
}

std::string DirectBindingGenerator::GenerateVectorBinding(const STLTypeInfo& info) {
    CodeBuilder builder;
    
    builder.AddIndentedLine("lua.new_usertype<" + info.full_type_name + ">(\"" + info.lua_type_name + "\"");
    builder.IncreaseIndent();
    builder.AddIndentedLine(", sol::constructors<" + info.full_type_name + "()>()");
    builder.AddIndentedLine(", \"size\", [](const " + info.full_type_name + "& vec) { return vec.size(); }");
    builder.AddIndentedLine(", \"empty\", [](const " + info.full_type_name + "& vec) { return vec.empty(); }");
    builder.AddIndentedLine(", \"clear\", []("+info.full_type_name + "& vec) { vec.clear(); }");
    
    // Extract element type for push_back
    std::string element_type = info.full_type_name;
    size_t start = element_type.find('<');
    size_t end = element_type.rfind('>');
    if (start != std::string::npos && end != std::string::npos) {
        element_type = element_type.substr(start + 1, end - start - 1);
    }
    
    builder.AddIndentedLine(", \"push_back\", []("+info.full_type_name + "& vec, const " + element_type + "& item) { vec.push_back(item); }");
    builder.AddIndentedLine(", \"pop_back\", []("+info.full_type_name + "& vec) { if(!vec.empty()) vec.pop_back(); }");
    
    // 添加索引访问方法
    builder.AddIndentedLine(", \"at\", [](const "+info.full_type_name + "& vec, size_t index) -> const " + element_type + "& { return vec.at(index); }");
    builder.AddIndentedLine(", \"get\", [](const "+info.full_type_name + "& vec, size_t index) -> sol::optional<" + element_type + "> {");
    builder.AddIndentedLine("        if (index < vec.size()) return vec[index];");
    builder.AddIndentedLine("        return sol::nullopt;");
    builder.AddIndentedLine("    }");
    builder.AddIndentedLine(", \"set\", []("+info.full_type_name + "& vec, size_t index, const " + element_type + "& value) {");
    builder.AddIndentedLine("        if (index < vec.size()) vec[index] = value;");
    builder.AddIndentedLine("    }");
    
    // 添加首尾访问方法
    builder.AddIndentedLine(", \"front\", [](const "+info.full_type_name + "& vec) -> const " + element_type + "& { return vec.front(); }");
    builder.AddIndentedLine(", \"back\", [](const "+info.full_type_name + "& vec) -> const " + element_type + "& { return vec.back(); }");
    
    // 添加容器操作方法
    builder.AddIndentedLine(", \"resize\", []("+info.full_type_name + "& vec, size_t new_size) { vec.resize(new_size); }");
    builder.AddIndentedLine(", \"reserve\", []("+info.full_type_name + "& vec, size_t capacity) { vec.reserve(capacity); }");
    builder.AddIndentedLine(", \"capacity\", [](const "+info.full_type_name + "& vec) { return vec.capacity(); }");
    
    // 添加查找和操作方法
    builder.AddIndentedLine(", \"insert\", []("+info.full_type_name + "& vec, size_t index, const " + element_type + "& item) {");
    builder.AddIndentedLine("        if (index <= vec.size()) vec.insert(vec.begin() + index, item);");
    builder.AddIndentedLine("    }");
    builder.AddIndentedLine(", \"erase\", []("+info.full_type_name + "& vec, size_t index) {");
    builder.AddIndentedLine("        if (index < vec.size()) vec.erase(vec.begin() + index);");
    builder.AddIndentedLine("    }");
    
    // 添加转换为Lua表的方法
    builder.AddIndentedLine(", \"to_table\", [](const "+info.full_type_name + "& vec, sol::this_state s) {");
    builder.AddIndentedLine("        sol::state_view lua(s);");
    builder.AddIndentedLine("        sol::table result = lua.create_table();");
    builder.AddIndentedLine("        for (size_t i = 0; i < vec.size(); ++i) {");
    builder.AddIndentedLine("            result[i + 1] = vec[i];");  // Lua is 1-indexed
    builder.AddIndentedLine("        }");
    builder.AddIndentedLine("        return result;");
    builder.AddIndentedLine("    }");
    builder.DecreaseIndent();
    builder.AddIndentedLine(");");
    
    return builder.Build();
}

std::string DirectBindingGenerator::GenerateMapBinding(const STLTypeInfo& info) {
    CodeBuilder builder;
    
    builder.AddIndentedLine("lua.new_usertype<" + info.full_type_name + ">(\"" + info.lua_type_name + "\"");
    builder.IncreaseIndent();
    builder.AddIndentedLine(", sol::constructors<" + info.full_type_name + "()>()");
    builder.AddIndentedLine(", \"size\", [](const " + info.full_type_name + "& map) { return map.size(); }");
    builder.AddIndentedLine(", \"empty\", [](const " + info.full_type_name + "& map) { return map.empty(); }");
    builder.AddIndentedLine(", \"clear\", []("+info.full_type_name + "& map) { map.clear(); }");
    
    // 提取键值类型
    std::string key_type, value_type;
    size_t start = info.full_type_name.find('<');
    size_t end = info.full_type_name.rfind('>');
    if (start != std::string::npos && end != std::string::npos) {
        std::string types_str = info.full_type_name.substr(start + 1, end - start - 1);
        size_t comma_pos = types_str.find(',');
        if (comma_pos != std::string::npos) {
            key_type = types_str.substr(0, comma_pos);
            value_type = types_str.substr(comma_pos + 1);
            
            // 清理空格
            key_type.erase(0, key_type.find_first_not_of(" \t"));
            key_type.erase(key_type.find_last_not_of(" \t") + 1);
            value_type.erase(0, value_type.find_first_not_of(" \t"));
            value_type.erase(value_type.find_last_not_of(" \t") + 1);
        }
    }
    
    if (!key_type.empty() && !value_type.empty()) {
        // Map 特有方法
        builder.AddIndentedLine(", \"set\", []("+info.full_type_name + "& map, const " + key_type + "& key, const " + value_type + "& value) { map[key] = value; }");
        builder.AddIndentedLine(", \"get\", [](const "+info.full_type_name + "& map, const " + key_type + "& key) -> sol::optional<" + value_type + "> {");
        builder.IncreaseIndent();
        builder.AddIndentedLine("auto it = map.find(key);");
        builder.AddIndentedLine("if (it != map.end()) return it->second;");
        builder.AddIndentedLine("return sol::nullopt;");
        builder.DecreaseIndent();
        builder.AddIndentedLine("}");
        builder.AddIndentedLine(", \"has\", [](const "+info.full_type_name + "& map, const " + key_type + "& key) { return map.find(key) != map.end(); }");
        builder.AddIndentedLine(", \"erase\", []("+info.full_type_name + "& map, const " + key_type + "& key) { return map.erase(key) > 0; }");
        builder.AddIndentedLine(", \"keys\", [](const "+info.full_type_name + "& map) {");
        builder.IncreaseIndent();
        builder.AddIndentedLine("std::vector<" + key_type + "> result;");
        builder.AddIndentedLine("for (const auto& pair : map) result.push_back(pair.first);");
        builder.AddIndentedLine("return result;");
        builder.DecreaseIndent();
        builder.AddIndentedLine("}");
        builder.AddIndentedLine(", \"values\", [](const "+info.full_type_name + "& map) {");
        builder.IncreaseIndent();
        builder.AddIndentedLine("std::vector<" + value_type + "> result;");
        builder.AddIndentedLine("for (const auto& pair : map) result.push_back(pair.second);");
        builder.AddIndentedLine("return result;");
        builder.DecreaseIndent();
        builder.AddIndentedLine("}");
    }
    
    builder.DecreaseIndent();
    builder.AddIndentedLine(");");
    
    return builder.Build();
}

std::string DirectBindingGenerator::GenerateSetBinding(const STLTypeInfo& info) {
    CodeBuilder builder;
    
    builder.AddIndentedLine("lua.new_usertype<" + info.full_type_name + ">(\"" + info.lua_type_name + "\"");
    builder.IncreaseIndent();
    builder.AddIndentedLine(", sol::constructors<" + info.full_type_name + "()>()");
    builder.AddIndentedLine(", \"size\", [](const " + info.full_type_name + "& set) { return set.size(); }");
    builder.AddIndentedLine(", \"empty\", [](const " + info.full_type_name + "& set) { return set.empty(); }");
    builder.AddIndentedLine(", \"clear\", []("+info.full_type_name + "& set) { set.clear(); }");
    
    // 提取元素类型
    std::string element_type = info.full_type_name;
    size_t start = element_type.find('<');
    size_t end = element_type.rfind('>');
    if (start != std::string::npos && end != std::string::npos) {
        element_type = element_type.substr(start + 1, end - start - 1);
    }
    
    if (!element_type.empty()) {
        // Set 特有方法
        builder.AddIndentedLine(", \"insert\", []("+info.full_type_name + "& set, const " + element_type + "& item) { return set.insert(item).second; }");
        builder.AddIndentedLine(", \"erase\", []("+info.full_type_name + "& set, const " + element_type + "& item) { return set.erase(item) > 0; }");
        builder.AddIndentedLine(", \"has\", [](const "+info.full_type_name + "& set, const " + element_type + "& item) { return set.find(item) != set.end(); }");
        builder.AddIndentedLine(", \"to_vector\", [](const "+info.full_type_name + "& set) {");
        builder.IncreaseIndent();
        builder.AddIndentedLine("std::vector<" + element_type + "> result(set.begin(), set.end());");
        builder.AddIndentedLine("return result;");
        builder.DecreaseIndent();
        builder.AddIndentedLine("}");
    }
    
    builder.DecreaseIndent();
    builder.AddIndentedLine(");");
    
    return builder.Build();
}

std::string DirectBindingGenerator::GenerateUnorderedMapBinding(const STLTypeInfo& info) {
    CodeBuilder builder;
    
    builder.AddIndentedLine("lua.new_usertype<" + info.full_type_name + ">(\"" + info.lua_type_name + "\"");
    builder.IncreaseIndent();
    builder.AddIndentedLine(", sol::constructors<" + info.full_type_name + "()>()");
    builder.AddIndentedLine(", \"size\", [](const " + info.full_type_name + "& map) { return map.size(); }");
    builder.AddIndentedLine(", \"empty\", [](const " + info.full_type_name + "& map) { return map.empty(); }");
    builder.AddIndentedLine(", \"clear\", []("+info.full_type_name + "& map) { map.clear(); }");
    
    // 提取键值类型
    std::string key_type, value_type;
    size_t start = info.full_type_name.find('<');
    size_t end = info.full_type_name.rfind('>');
    if (start != std::string::npos && end != std::string::npos) {
        std::string types_str = info.full_type_name.substr(start + 1, end - start - 1);
        size_t comma_pos = types_str.find(',');
        if (comma_pos != std::string::npos) {
            key_type = types_str.substr(0, comma_pos);
            value_type = types_str.substr(comma_pos + 1);
            
            // 清理空格
            key_type.erase(0, key_type.find_first_not_of(" \t"));
            key_type.erase(key_type.find_last_not_of(" \t") + 1);
            value_type.erase(0, value_type.find_first_not_of(" \t"));
            value_type.erase(value_type.find_last_not_of(" \t") + 1);
        }
    }
    
    if (!key_type.empty() && !value_type.empty()) {
        // UnorderedMap 方法（与Map相同）
        builder.AddIndentedLine(", \"set\", []("+info.full_type_name + "& map, const " + key_type + "& key, const " + value_type + "& value) { map[key] = value; }");
        builder.AddIndentedLine(", \"get\", [](const "+info.full_type_name + "& map, const " + key_type + "& key) -> sol::optional<" + value_type + "> {");
        builder.IncreaseIndent();
        builder.AddIndentedLine("auto it = map.find(key);");
        builder.AddIndentedLine("if (it != map.end()) return it->second;");
        builder.AddIndentedLine("return sol::nullopt;");
        builder.DecreaseIndent();
        builder.AddIndentedLine("}");
        builder.AddIndentedLine(", \"has\", [](const "+info.full_type_name + "& map, const " + key_type + "& key) { return map.find(key) != map.end(); }");
        builder.AddIndentedLine(", \"erase\", []("+info.full_type_name + "& map, const " + key_type + "& key) { return map.erase(key) > 0; }");
        builder.AddIndentedLine(", \"keys\", [](const "+info.full_type_name + "& map) {");
        builder.IncreaseIndent();
        builder.AddIndentedLine("std::vector<" + key_type + "> result;");
        builder.AddIndentedLine("for (const auto& pair : map) result.push_back(pair.first);");
        builder.AddIndentedLine("return result;");
        builder.DecreaseIndent();
        builder.AddIndentedLine("}");
        builder.AddIndentedLine(", \"values\", [](const "+info.full_type_name + "& map) {");
        builder.IncreaseIndent();
        builder.AddIndentedLine("std::vector<" + value_type + "> result;");
        builder.AddIndentedLine("for (const auto& pair : map) result.push_back(pair.second);");
        builder.AddIndentedLine("return result;");
        builder.DecreaseIndent();
        builder.AddIndentedLine("}");
    }
    
    builder.DecreaseIndent();
    builder.AddIndentedLine(");");
    
    return builder.Build();
}

std::string DirectBindingGenerator::GenerateListBinding(const STLTypeInfo& info) {
    CodeBuilder builder;
    
    builder.AddIndentedLine("lua.new_usertype<" + info.full_type_name + ">(\"" + info.lua_type_name + "\"");
    builder.IncreaseIndent();
    builder.AddIndentedLine(", sol::constructors<" + info.full_type_name + "()>()");
    builder.AddIndentedLine(", \"size\", [](const " + info.full_type_name + "& list) { return list.size(); }");
    builder.AddIndentedLine(", \"empty\", [](const " + info.full_type_name + "& list) { return list.empty(); }");
    builder.AddIndentedLine(", \"clear\", []("+info.full_type_name + "& list) { list.clear(); }");
    
    // 提取元素类型
    std::string element_type = info.full_type_name;
    size_t start = element_type.find('<');
    size_t end = element_type.rfind('>');
    if (start != std::string::npos && end != std::string::npos) {
        element_type = element_type.substr(start + 1, end - start - 1);
    }
    
    if (!element_type.empty()) {
        // List 特有方法
        builder.AddIndentedLine(", \"push_back\", []("+info.full_type_name + "& list, const " + element_type + "& item) { list.push_back(item); }");
        builder.AddIndentedLine(", \"pop_back\", []("+info.full_type_name + "& list) { if(!list.empty()) list.pop_back(); }");
        builder.AddIndentedLine(", \"push_front\", []("+info.full_type_name + "& list, const " + element_type + "& item) { list.push_front(item); }");
        builder.AddIndentedLine(", \"pop_front\", []("+info.full_type_name + "& list) { if(!list.empty()) list.pop_front(); }");
        builder.AddIndentedLine(", \"front\", [](const "+info.full_type_name + "& list) -> sol::optional<" + element_type + "> {");
        builder.IncreaseIndent();
        builder.AddIndentedLine("if(!list.empty()) return list.front();");
        builder.AddIndentedLine("return sol::nullopt;");
        builder.DecreaseIndent();
        builder.AddIndentedLine("}");
        builder.AddIndentedLine(", \"back\", [](const "+info.full_type_name + "& list) -> sol::optional<" + element_type + "> {");
        builder.IncreaseIndent();
        builder.AddIndentedLine("if(!list.empty()) return list.back();");
        builder.AddIndentedLine("return sol::nullopt;");
        builder.DecreaseIndent();
        builder.AddIndentedLine("}");
    }
    
    builder.DecreaseIndent();
    builder.AddIndentedLine(");");
    
    return builder.Build();
}

std::string DirectBindingGenerator::GetLuaTypeName(const std::string& cpp_type) {
    // 简单的类型名转换
    std::string result = cpp_type;
    std::replace(result.begin(), result.end(), ':', '_');
    std::replace(result.begin(), result.end(), '<', '_');
    std::replace(result.begin(), result.end(), '>', '_');
    std::replace(result.begin(), result.end(), ' ', '_');
    return result;
}

std::string DirectBindingGenerator::GetQualifiedTypeName(const ExportInfo& info) {
    if (!info.qualified_name.empty()) {
        return info.qualified_name;
    }
    
    // 构建合格名称
    std::string result;
    if (!info.namespace_name.empty() && info.namespace_name != "global") {
        result = info.namespace_name + "::";
    }
    
    if (!info.parent_class.empty()) {
        result += info.parent_class + "::";
    }
    
    result += info.name;
    return result;
}

bool DirectBindingGenerator::IsSmartPointer(const std::string& type_name) {
    return type_name.find("std::shared_ptr") == 0 ||
           type_name.find("std::unique_ptr") == 0 ||
           type_name.find("std::weak_ptr") == 0;
}

std::string DirectBindingGenerator::GenerateSmartPointerBinding(const ExportInfo& info) {
    // TODO: 实现智能指针绑定
    return "// TODO: Smart pointer binding for " + info.type_name;
}

std::unordered_map<std::string, std::vector<ExportInfo>> 
DirectBindingGenerator::GroupExportsByType(const std::vector<ExportInfo>& export_items) {
    std::unordered_map<std::string, std::vector<ExportInfo>> grouped;
    
    for (const auto& item : export_items) {
        grouped[item.export_type].push_back(item);
    }
    
    return grouped;
}

bool DirectBindingGenerator::ValidateExportInfo(const ExportInfo& info, std::vector<std::string>& errors) {
    bool valid = true;
    
    if (info.name.empty()) {
        errors.push_back("Export item has empty name");
        valid = false;
    }
    
    if (info.export_type.empty()) {
        errors.push_back("Export item '" + info.name + "' has empty export type");
        valid = false;
    }
    
    return valid;
}

std::string DirectBindingGenerator::GenerateErrorHandling() {
    // TODO: 实现错误处理代码生成
    return "";
}

} // namespace lua_binding_generator