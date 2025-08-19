/**
 * @file main.cpp
 * @brief lua_binding_generator 主程序入口
 * 
 * 零配置 C++ 到 Lua 绑定代码生成工具
 */

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <typeinfo>
#include <stdexcept>

#include "compiler_detector.h"
#include "dynamic_compilation_database.h"
#include "ast_visitor.h"
#include "direct_binding_generator.h"
#include "logger.h"

#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/Diagnostic.h>

using namespace lua_binding_generator;
namespace fs = std::filesystem;

/**
 * @brief 将 ExportInfo::Type 枚举转换为字符串
 */
std::string typeToString(ExportInfo::Type type) {
    switch (type) {
        case ExportInfo::Type::Class: return "class";
        case ExportInfo::Type::Method: return "method";
        case ExportInfo::Type::StaticMethod: return "static_method";
        case ExportInfo::Type::Constructor: return "constructor";
        case ExportInfo::Type::Property: return "property";
        case ExportInfo::Type::Function: return "function";
        case ExportInfo::Type::Enum: return "enum";
        case ExportInfo::Type::Constant: return "constant";
        case ExportInfo::Type::Namespace: return "namespace";
        case ExportInfo::Type::Module: return "module";
        case ExportInfo::Type::Operator: return "operator";
        case ExportInfo::Type::TypeConverter: return "type_converter";
        case ExportInfo::Type::Inherit: return "inherit";
        case ExportInfo::Type::STLContainer: return "stl_container";
        default: return "unknown";
    }
}

/**
 * @brief 打印导出项的详细信息（调试用）
 */
void logExportItemDetails(const ExportInfo& item, int index) {
    std::cout << "🔍 [DEBUG] 导出项 #" << index << ":" << std::endl;
    std::cout << "   类型: " << typeToString(item.type) << std::endl;
    std::cout << "   名称: " << item.name << std::endl;
    std::cout << "   Lua名称: " << item.lua_name << std::endl;
    std::cout << "   命名空间: " << item.namespace_name << std::endl;
    std::cout << "   完全限定名: " << item.qualified_name << std::endl;
    std::cout << "   源码位置: " << item.source_location << std::endl;
    std::cout << "   所属类: " << item.owner_class << std::endl;
    std::cout << "   父类: " << item.parent_class << std::endl;
    std::cout << "   参数类型数量: " << item.parameter_types.size() << std::endl;
    std::cout << "   返回类型: " << item.return_type << std::endl;
}

/**
 * @brief 验证导出项数据完整性
 */
bool validateExportItem(const ExportInfo& item, int index) {
    bool isValid = true;
    
    if (item.name.empty()) {
        std::cerr << "❌ [验证] 导出项 #" << index << " 名称为空" << std::endl;
        isValid = false;
    }
    
    // 更智能的命名空间验证：只对真正异常的情况报告警告
    if (item.namespace_name.empty() && item.type != ExportInfo::Type::Module) {
        bool shouldWarn = false;
        
        // 对于类成员（方法、属性、构造函数、操作符），如果有parent_class，命名空间可以为空
        if ((item.type == ExportInfo::Type::Method || 
             item.type == ExportInfo::Type::Property || 
             item.type == ExportInfo::Type::Constructor || 
             item.type == ExportInfo::Type::StaticMethod ||
             item.type == ExportInfo::Type::Operator) && 
             !item.parent_class.empty()) {
            // 类成员不需要独立的命名空间验证
            shouldWarn = false;
        }
        // 对于类，检查是否在明显的命名空间限定名中
        else if (item.type == ExportInfo::Type::Class) {
            // 如果限定名包含::，说明确实在命名空间中，但推导可能失败了
            if (!item.qualified_name.empty() && item.qualified_name.find("::") != std::string::npos) {
                // 进一步检查是否是标准库命名空间
                if (item.qualified_name.find("std::") == 0) {
                    shouldWarn = false; // std命名空间不警告
                } else {
                    shouldWarn = true; // 其他命名空间的类应该有命名空间信息
                }
            }
        }
        // 对于全局函数，只有在明确有命名空间限定时才警告
        else if (item.type == ExportInfo::Type::Function) {
            if (!item.qualified_name.empty() && item.qualified_name.find("::") != std::string::npos) {
                // 排除标准库函数
                if (item.qualified_name.find("std::") != 0) {
                    shouldWarn = true;
                }
            }
        }
        // 对于常量和变量，同样检查限定名
        else if (item.type == ExportInfo::Type::Constant || item.type == ExportInfo::Type::Enum) {
            if (!item.qualified_name.empty() && item.qualified_name.find("::") != std::string::npos) {
                // 排除标准库
                if (item.qualified_name.find("std::") != 0) {
                    shouldWarn = true;
                }
            }
        }
        
        if (shouldWarn) {
            std::cerr << "⚠️  [验证] 导出项 #" << index << " (" << item.name << ") 可能缺少命名空间信息" << std::endl;
        }
    }
    
    if (item.return_type.empty() && item.type == ExportInfo::Type::Function) {
        std::cerr << "⚠️  [验证] 函数导出项 #" << index << " (" << item.name << ") 返回类型为空" << std::endl;
    }
    
    return isValid;
}

/**
 * @brief 记录系统状态信息
 */
void logSystemState(const std::string& step) {
    auto current_time = std::chrono::steady_clock::now();
    auto time_since_epoch = current_time.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch).count();
    
    std::cout << "📊 [状态] " << step << " (时间戳: " << millis << ")" << std::endl;
    
    // 打印内存使用情况（如果可能）
    try {
        auto path = fs::current_path();
        std::cout << "   当前目录: " << path << std::endl;
    } catch (...) {
        std::cout << "   无法获取当前目录" << std::endl;
    }
}

/**
 * @brief AST消费器，用于运行LuaASTVisitor
 */
class LuaBindingASTConsumer : public clang::ASTConsumer {
public:
    explicit LuaBindingASTConsumer(std::vector<ExportInfo>* export_infos, const std::string& debug_log_path = "", const std::string& stats_log_path = "")
        : export_infos_(export_infos), visitor_(nullptr), debug_log_path_(debug_log_path), stats_log_path_(stats_log_path) {}

    void Initialize(clang::ASTContext& Context) override {
        visitor_ = std::make_unique<LuaASTVisitor>(&Context);
        
        // 初始化日志系统
        if (!debug_log_path_.empty() || !stats_log_path_.empty()) {
            std::string debug_path = debug_log_path_.empty() ? "ast_debug.log" : debug_log_path_;
            std::string stats_path = stats_log_path_.empty() ? "ast_stats.log" : stats_log_path_;
            visitor_->InitializeLogging(debug_path, stats_path);
        }
    }

    void HandleTranslationUnit(clang::ASTContext& Context) override {
        if (visitor_) {
            visitor_->TraverseDecl(Context.getTranslationUnitDecl());
            
            // 生成统计报告
            visitor_->GenerateStatisticsReport();
            
            // 收集找到的导出项
            const auto& found_items = visitor_->GetExportedItems();
            export_infos_->insert(export_infos_->end(), found_items.begin(), found_items.end());
            
            // 关闭日志系统
            visitor_->CloseLogging();
        }
    }

private:
    std::vector<ExportInfo>* export_infos_;
    std::unique_ptr<LuaASTVisitor> visitor_;
    std::string debug_log_path_;
    std::string stats_log_path_;
};

/**
 * @brief 前端动作，用于创建AST消费器
 */
class LuaBindingFrontendAction : public clang::ASTFrontendAction {
public:
    explicit LuaBindingFrontendAction(std::vector<ExportInfo>* export_infos, const std::string& debug_log_path = "", const std::string& stats_log_path = "")
        : export_infos_(export_infos), debug_log_path_(debug_log_path), stats_log_path_(stats_log_path) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& CI, llvm::StringRef file) override {
        return std::make_unique<LuaBindingASTConsumer>(export_infos_, debug_log_path_, stats_log_path_);
    }

private:
    std::vector<ExportInfo>* export_infos_;
    std::string debug_log_path_;
    std::string stats_log_path_;
};

/**
 * @brief 前端动作工厂
 */
class LuaBindingActionFactory : public clang::tooling::FrontendActionFactory {
public:
    explicit LuaBindingActionFactory(std::vector<ExportInfo>* export_infos, const std::string& debug_log_path = "", const std::string& stats_log_path = "")
        : export_infos_(export_infos), debug_log_path_(debug_log_path), stats_log_path_(stats_log_path) {}

    std::unique_ptr<clang::FrontendAction> create() override {
        return std::make_unique<LuaBindingFrontendAction>(export_infos_, debug_log_path_, stats_log_path_);
    }

private:
    std::vector<ExportInfo>* export_infos_;
    std::string debug_log_path_;
    std::string stats_log_path_;
};

/**
 * @brief 程序选项
 */
struct ProgramOptions {
    std::vector<std::string> input_files;
    std::string output_dir = "generated_bindings";
    std::string input_dir;
    std::vector<std::string> exclude_files;
    std::vector<std::string> include_paths;
    std::string module_name;
    std::string compiler_path;
    std::string log_file;
    bool verbose = false;
    bool show_help = false;
    bool show_stats = false;
    bool force_rebuild = false;
};

/**
 * @brief 显示帮助信息
 */
void showHelp(const char* program_name) {
    std::cout << "Lua Binding Generator - 零配置 C++ 到 Lua 绑定工具\n\n";
    std::cout << "用法:\n";
    std::cout << "  " << program_name << " file1.h file2.h ... [选项]\n";
    std::cout << "  " << program_name << " --input_dir=<目录> [选项]\n\n";
    std::cout << "选项:\n";
    std::cout << "  --help, -h              显示此帮助信息\n";
    std::cout << "  --verbose, -v           启用详细输出\n";
    std::cout << "  --output_dir=<目录>     输出目录 (默认: generated_bindings)\n";
    std::cout << "  --input_dir=<目录>      输入目录（递归搜索 .h 文件）\n";
    std::cout << "  --exclude_files=<列表>  排除文件列表（逗号分隔）\n";
    std::cout << "  --include=<路径>        额外的包含路径\n";
    std::cout << "  --module-name=<名称>    模块名称\n";
    std::cout << "  --compiler=<路径>       指定编译器路径\n";
    std::cout << "  --log-file=<文件>       日志文件路径\n";
    std::cout << "  --show-stats            显示生成统计信息\n";
    std::cout << "  --force-rebuild         强制重新构建\n\n";
    std::cout << "示例:\n";
    std::cout << "  " << program_name << " examples/real_test.h --output_dir=bindings\n";
    std::cout << "  " << program_name << " --input_dir=src/game --exclude_files=internal.h,debug.h\n";
    std::cout << "  " << program_name << " game.h player.h --module-name=GameCore --verbose\n\n";
    std::cout << "注意:\n";
    std::cout << "  此工具需要 C++ 编译环境，支持 Clang、GCC 或 MSVC。\n";
    std::cout << "  如果未检测到编译器，请确保已安装并在 PATH 环境变量中。\n";
}

/**
 * @brief 解析命令行参数
 */
ProgramOptions parseCommandLine(int argc, char* argv[]) {
    ProgramOptions options;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            options.show_help = true;
        }
        else if (arg == "--verbose" || arg == "-v") {
            options.verbose = true;
        }
        else if (arg.substr(0, 13) == "--output_dir=") {
            options.output_dir = arg.substr(13);
        }
        else if (arg.substr(0, 12) == "--input_dir=") {
            options.input_dir = arg.substr(12);
        }
        else if (arg.substr(0, 16) == "--exclude_files=") {
            std::string files = arg.substr(16);
            size_t pos = 0;
            while ((pos = files.find(',')) != std::string::npos) {
                options.exclude_files.push_back(files.substr(0, pos));
                files.erase(0, pos + 1);
            }
            if (!files.empty()) {
                options.exclude_files.push_back(files);
            }
        }
        else if (arg.substr(0, 10) == "--include=") {
            options.include_paths.push_back(arg.substr(10));
        }
        else if (arg.substr(0, 14) == "--module-name=") {
            options.module_name = arg.substr(14);
        }
        else if (arg.substr(0, 11) == "--compiler=") {
            options.compiler_path = arg.substr(11);
        }
        else if (arg.substr(0, 11) == "--log-file=") {
            options.log_file = arg.substr(11);
        }
        else if (arg == "--show-stats") {
            options.show_stats = true;
        }
        else if (arg == "--force-rebuild") {
            options.force_rebuild = true;
        }
        else if (arg.substr(0, 2) != "--") {
            // 输入文件
            options.input_files.push_back(arg);
        }
        else {
            std::cerr << "未知选项: " << arg << std::endl;
            options.show_help = true;
        }
    }
    
    return options;
}

/**
 * @brief 收集输入文件
 */
std::vector<std::string> collectInputFiles(const ProgramOptions& options) {
    std::vector<std::string> files = options.input_files;
    
    // 如果指定了输入目录，递归搜索 .h 文件
    if (!options.input_dir.empty()) {
        if (!fs::exists(options.input_dir)) {
            throw std::runtime_error("输入目录不存在: " + options.input_dir);
        }
        
        for (const auto& entry : fs::recursive_directory_iterator(options.input_dir)) {
            if (entry.is_regular_file()) {
                std::string path = entry.path().string();
                if ((path.size() >= 2 && path.substr(path.size()-2) == ".h") || 
                    (path.size() >= 4 && path.substr(path.size()-4) == ".hpp")) {
                    // 检查是否在排除列表中
                    std::string filename = entry.path().filename().string();
                    bool excluded = false;
                    for (const auto& exclude : options.exclude_files) {
                        if (filename == exclude) {
                            excluded = true;
                            break;
                        }
                    }
                    if (!excluded) {
                        files.push_back(path);
                    }
                }
            }
        }
    }
    
    // 验证所有文件存在
    for (const auto& file : files) {
        fs::path file_path(file);
        
        // 尝试相对路径和绝对路径
        bool file_exists = false;
        
        if (fs::exists(file_path)) {
            file_exists = true;
        } else if (!file_path.is_absolute()) {
            // 如果是相对路径，也尝试从当前工作目录查找
            auto abs_path = fs::absolute(file_path);
            if (fs::exists(abs_path)) {
                file_exists = true;
            }
        }
        
        if (!file_exists) {
            std::cerr << "警告: 文件不存在: " << file << std::endl;
            // 尝试提供一些调试信息
            std::cerr << "   当前工作目录: " << fs::current_path() << std::endl;
            if (!file_path.is_absolute()) {
                std::cerr << "   尝试绝对路径: " << fs::absolute(file_path) << std::endl;
            }
        }
    }
    
    return files;
}

/**
 * @brief 主程序
 */
int main(int argc, char* argv[]) {
    try {
        // 解析命令行参数
        ProgramOptions options = parseCommandLine(argc, argv);
        
        if (options.show_help || options.input_files.empty() && options.input_dir.empty()) {
            showHelp(argv[0]);
            return options.show_help ? 0 : 1;
        }
        
        // 初始化日志系统
        if (options.verbose) {
            Logger::info("Verbose mode enabled");
        }
        if (!options.log_file.empty()) {
            Logger::setLogFile(options.log_file);
        }
        
        // 记录开始时间
        auto start_time = std::chrono::steady_clock::now();
        
        std::cout << "🔍 开始分析源文件..." << std::endl;
        
        // 收集输入文件
        std::vector<std::string> input_files = collectInputFiles(options);
        if (input_files.empty()) {
            std::cerr << "错误: 未指定源文件" << std::endl;
            showHelp(argv[0]);
            return 1;
        }
        
        std::cout << "已配置 " << input_files.size() << " 个源文件" << std::endl;
        
        // 检测编译器
        Logger::debug("开始检测编译器...");
        CompilerDetector detector;
        auto compiler_info = detector.DetectCompiler();
        
        if (!compiler_info.IsValid()) {
            std::cerr << "错误: 无法检测到 C++ 编译器" << std::endl;
            std::cerr << "请确保已安装 Clang、GCC 或 MSVC 并在 PATH 中" << std::endl;
            return 1;
        }
        
        std::cout << "🔧 检测到编译器: " << compiler_info.type << " " << compiler_info.version << std::endl;
        std::cout << "📍 编译器路径: " << compiler_info.compiler_path << std::endl;
        std::cout << "📦 系统包含路径: " << compiler_info.include_paths.size() << " 个" << std::endl;
        
        // 创建编译数据库
        std::string zeus_include_path = ""; // 空字符串，因为这不是Zeus项目
        std::vector<std::string> additional_flags;
        
        // 添加额外的包含路径到编译标志
        for (const auto& path : options.include_paths) {
            additional_flags.push_back("-I" + path);
        }
        
        DynamicCompilationDatabase compile_db(input_files, zeus_include_path, compiler_info, additional_flags);
        
        if (options.verbose) {
            std::cout << "🗃️  编译数据库信息:" << std::endl;
            std::cout << "已配置 " << input_files.size() << " 个源文件" << std::endl;
        }
        
        // 创建 AST 访问器并分析源文件
        std::cout << "🔍 开始分析源文件..." << std::endl;
        
        // 准备日志文件路径
        std::string debug_log_path = options.output_dir + "/ast_debug.log";
        std::string stats_log_path = options.output_dir + "/ast_stats.log";
        
        // 创建输出目录（如果不存在）
        if (!fs::exists(options.output_dir)) {
            Logger::info("创建输出目录: " + options.output_dir);
            fs::create_directories(options.output_dir);
        }
        
        std::cout << "📝 日志文件: " << debug_log_path << std::endl;
        std::cout << "📊 统计文件: " << stats_log_path << std::endl;
        
        // 使用Clang工具分析源文件
        std::vector<ExportInfo> export_infos;
        
        try {
            std::cout << "🔧 创建Clang分析工具..." << std::endl;
            
            // 创建Clang工具
            clang::tooling::ClangTool tool(compile_db, input_files);
            
            std::cout << "⚙️  创建AST访问器..." << std::endl;
            
            // 创建动作工厂
            auto action_factory = std::make_unique<LuaBindingActionFactory>(&export_infos, debug_log_path, stats_log_path);
            
            std::cout << "🚀 开始AST分析..." << std::endl;
            
            // 运行工具
            int result = tool.run(action_factory.get());
            
            std::cout << "✅ AST分析完成，结果代码: " << result << std::endl;
            
            if (result != 0) {
                std::cerr << "⚠️  AST分析过程中出现错误" << std::endl;
                Logger::debug("Clang工具返回错误代码: " + std::to_string(result));
            }
            
        } catch (const std::exception& e) {
            std::cerr << "AST分析异常: " << e.what() << std::endl;
            Logger::debug("AST分析异常详情: " + std::string(e.what()));
        }
        
        Logger::info("✓ AST 处理完成，共处理 " + std::to_string(input_files.size()) + " 个文件，发现 " + std::to_string(export_infos.size()) + " 个导出项");
        
        std::cout << "📊 发现 " << export_infos.size() << " 个导出项" << std::endl;
        
        if (export_infos.empty()) {
            std::cout << "⚠️  未发现任何导出项，请检查是否正确使用了 EXPORT_LUA_* 宏" << std::endl;
            return 0;
        }
        
        std::cout << "✅ 解析完成！共发现 " << export_infos.size() << " 个导出项" << std::endl;
        
        // 生成绑定代码
        std::cout << "🔄 开始代码生成..." << std::endl;
        logSystemState("代码生成阶段开始");
        
        // 验证导出数据
        std::cout << "🔍 验证导出数据完整性..." << std::endl;
        std::cout << "📊 待处理导出项: " << export_infos.size() << " 个" << std::endl;
        
        // 详细验证每个导出项
        int invalid_count = 0;
        for (size_t i = 0; i < export_infos.size(); ++i) {
            if (!validateExportItem(export_infos[i], static_cast<int>(i))) {
                invalid_count++;
            }
        }
        
        if (invalid_count > 0) {
            std::cerr << "❌ 发现 " << invalid_count << " 个无效的导出项" << std::endl;
        } else {
            std::cout << "✅ 所有导出项验证通过" << std::endl;
        }
        
        // 类型统计
        std::cout << "🗂️  导出项类型统计:" << std::endl;
        std::map<std::string, int> type_count;
        for (const auto& info : export_infos) {
            type_count[typeToString(info.type)]++;
        }
        for (const auto& [type, count] : type_count) {
            std::cout << "   - " << type << ": " << count << " 个" << std::endl;
        }
        
        // 如果启用详细模式，打印前几个导出项的详细信息
        if (options.verbose && !export_infos.empty()) {
            std::cout << "🔍 [DEBUG] 显示前5个导出项详情:" << std::endl;
            for (size_t i = 0; i < std::min(size_t(5), export_infos.size()); ++i) {
                logExportItemDetails(export_infos[i], static_cast<int>(i));
            }
        }
        
        logSystemState("准备创建代码生成器");
        std::cout << "⚙️  创建代码生成器..." << std::endl;
        
        DirectBindingGenerator generator;
        std::cout << "✅ 代码生成器对象创建成功" << std::endl;
        
        DirectBindingGenerator::GenerationOptions gen_options;
        gen_options.output_directory = options.output_dir;
        // 注意：这个结构体没有module_name和verbose字段
        
        std::cout << "🔧 配置生成器选项..." << std::endl;
        std::cout << "   输出目录: " << gen_options.output_directory << std::endl;
        
        try {
            generator.SetOptions(gen_options);
            std::cout << "✅ 生成器选项配置成功" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "❌ 配置生成器选项失败: " << e.what() << std::endl;
            throw;
        }
        
        // 创建输出目录
        std::cout << "📁 检查输出目录: " << options.output_dir << std::endl;
        if (!fs::exists(options.output_dir)) {
            Logger::info("创建输出目录: " + options.output_dir);
            try {
                fs::create_directories(options.output_dir);
                std::cout << "✅ 输出目录创建成功" << std::endl;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "❌ 创建输出目录失败: " << e.what() << std::endl;
                throw;
            }
        } else {
            std::cout << "✅ 输出目录已存在" << std::endl;
        }
        
        // 使用正确的API生成绑定
        std::string module_name = options.module_name.empty() ? "MyProject" : options.module_name;
        std::cout << "📦 生成模块: " << module_name << std::endl;
        std::cout << "📁 输出目录: " << options.output_dir << std::endl;
        std::cout << "📊 输入数据量: " << export_infos.size() << " 个导出项" << std::endl;
        
        // 绑定生成的具体错误处理
        decltype(generator.GenerateModuleBinding(module_name, export_infos)) result;
        
        logSystemState("开始调用GenerateModuleBinding");
        
        try {
            std::cout << "⚡ 正在生成绑定代码..." << std::endl;
            std::cout << "🔍 [DEBUG] 调用 GenerateModuleBinding(\"" << module_name << "\", " << export_infos.size() << " items)" << std::endl;
            
            auto start_time = std::chrono::steady_clock::now();
            result = generator.GenerateModuleBinding(module_name, export_infos);
            auto end_time = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            std::cout << "✅ 绑定生成完成 (耗时: " << duration.count() << "ms)" << std::endl;
            std::cout << "🔍 [DEBUG] GenerateModuleBinding 返回结果: success=" << result.success << std::endl;
            
        } catch (const std::runtime_error& e) {
            std::cerr << "❌ 绑定生成运行时错误: " << e.what() << std::endl;
            logSystemState("运行时错误发生时的状态");
            Logger::debug("绑定生成运行时错误详情: " + std::string(e.what()));
            std::cerr << "🔍 错误上下文:" << std::endl;
            std::cerr << "   - 模块名: " << module_name << std::endl;
            std::cerr << "   - 输出目录: " << options.output_dir << std::endl;
            std::cerr << "   - 导出项数量: " << export_infos.size() << std::endl;
            throw;
        } catch (const std::logic_error& e) {
            std::cerr << "❌ 绑定生成逻辑错误: " << e.what() << std::endl;
            logSystemState("逻辑错误发生时的状态");
            Logger::debug("绑定生成逻辑错误详情: " + std::string(e.what()));
            std::cerr << "🔍 错误上下文:" << std::endl;
            std::cerr << "   - 模块名: " << module_name << std::endl;
            std::cerr << "   - 输出目录: " << options.output_dir << std::endl;
            std::cerr << "   - 导出项数量: " << export_infos.size() << std::endl;
            throw;
        } catch (const std::bad_alloc& e) {
            std::cerr << "❌ 绑定生成内存分配失败: " << e.what() << std::endl;
            logSystemState("内存分配错误发生时的状态");
            std::cerr << "💡 可能原因: 导出项过多(" << export_infos.size() << "个)，请尝试分批处理" << std::endl;
            std::cerr << "🔍 内存相关信息:" << std::endl;
            std::cerr << "   - 导出项数量: " << export_infos.size() << std::endl;
            std::cerr << "   - 预计内存需求: ~" << (export_infos.size() * 1024) << " bytes" << std::endl;
            throw;
        } catch (const std::exception& e) {
            std::cerr << "❌ 绑定生成标准异常: " << e.what() << std::endl;
            logSystemState("标准异常发生时的状态");
            Logger::debug("绑定生成异常详情: " + std::string(e.what()));
            std::cerr << "🔍 异常类型信息:" << std::endl;
            std::cerr << "   - 错误信息: " << e.what() << std::endl;
            std::cerr << "   - 模块名: " << module_name << std::endl;
            std::cerr << "   - 导出项数量: " << export_infos.size() << std::endl;
            throw;
        } catch (...) {
            std::cerr << "❌ 绑定生成过程中发生未知异常" << std::endl;
            logSystemState("未知异常发生时的状态");
            std::cerr << "🔍 详细调试信息:" << std::endl;
            std::cerr << "   - 模块名: \"" << module_name << "\"" << std::endl;
            std::cerr << "   - 导出项数量: " << export_infos.size() << std::endl;
            std::cerr << "   - 输出目录: \"" << options.output_dir << "\"" << std::endl;
            std::cerr << "   - 工作目录: " << fs::current_path() << std::endl;
            
            // 显示最近几个导出项的信息
            if (!export_infos.empty()) {
                std::cerr << "🔍 最后几个导出项信息:" << std::endl;
                size_t start = export_infos.size() > 3 ? export_infos.size() - 3 : 0;
                for (size_t i = start; i < export_infos.size(); ++i) {
                    std::cerr << "   [" << i << "] " << typeToString(export_infos[i].type) 
                              << ": " << export_infos[i].name << std::endl;
                }
            }
            
            std::cerr << "💡 可能的原因:" << std::endl;
            std::cerr << "   - DirectBindingGenerator内部错误" << std::endl;
            std::cerr << "   - 模板实例化失败" << std::endl;
            std::cerr << "   - 代码生成缓冲区溢出" << std::endl;
            std::cerr << "   - 内存访问越界" << std::endl;
            std::cerr << "   - C++异常处理机制失效" << std::endl;
            throw;
        }
        
        std::vector<std::string> generated_files;
        if (result.success) {
            std::string output_file = options.output_dir + "/" + module_name + "_bindings.cpp";
            std::ofstream file(output_file);
            file << result.generated_code;
            file.close();
            generated_files.push_back(output_file);
        }
        
        std::cout << "🔄 已转换 " << export_infos.size() << " 个导出项" << std::endl;
        std::cout << "✅ 代码生成完成！" << std::endl;
        
        // 输出结果
        for (const auto& file : generated_files) {
            std::cout << "📄 输出文件: " << file << std::endl;
        }
        
        // 显示统计信息
        if (options.show_stats || options.verbose) {
            std::cout << "📊 生成了 " << result.total_bindings << " 个绑定" << std::endl;
            if (!result.warnings.empty()) {
                std::cout << "⚠️  警告 " << result.warnings.size() << " 个" << std::endl;
            }
            if (!result.errors.empty()) {
                std::cout << "❌ 错误 " << result.errors.size() << " 个" << std::endl;
            }
        }
        
        // 计算耗时
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "🎉 总耗时: " << duration.count() << " 毫秒" << std::endl;
        
        return 0;
        
    } catch (const std::runtime_error& e) {
        std::cerr << "❌ 运行时错误: " << e.what() << std::endl;
        return 1;
    } catch (const std::logic_error& e) {
        std::cerr << "❌ 逻辑错误: " << e.what() << std::endl;
        return 1;
    } catch (const std::bad_alloc& e) {
        std::cerr << "❌ 内存分配失败: " << e.what() << std::endl;
        std::cerr << "💡 请检查可用内存或减少输入文件大小" << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "❌ 标准异常: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ 发生未知类型的异常" << std::endl;
        std::cerr << "💡 可能的原因:" << std::endl;
        std::cerr << "   - Clang内部异常" << std::endl;
        std::cerr << "   - 系统级别异常" << std::endl;
        std::cerr << "   - 第三方库异常" << std::endl;
        std::cerr << "🔍 请查看生成的日志文件获取更多信息" << std::endl;
        return 1;
    }
}