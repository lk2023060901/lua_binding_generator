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

#include "compiler_detector.h"
#include "dynamic_compilation_database.h"
#include "ast_visitor.h"
#include "direct_binding_generator.h"
#include "logger.h"

#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>

using namespace lua_binding_generator;
namespace fs = std::filesystem;

/**
 * @brief AST消费器，用于运行LuaASTVisitor
 */
class LuaBindingASTConsumer : public clang::ASTConsumer {
public:
    explicit LuaBindingASTConsumer(std::vector<ExportInfo>* export_infos)
        : export_infos_(export_infos), visitor_(nullptr) {}

    void Initialize(clang::ASTContext& Context) override {
        visitor_ = std::make_unique<LuaASTVisitor>(&Context);
    }

    void HandleTranslationUnit(clang::ASTContext& Context) override {
        if (visitor_) {
            visitor_->TraverseDecl(Context.getTranslationUnitDecl());
            
            // 收集找到的导出项
            const auto& found_items = visitor_->GetExportedItems();
            export_infos_->insert(export_infos_->end(), found_items.begin(), found_items.end());
        }
    }

private:
    std::vector<ExportInfo>* export_infos_;
    std::unique_ptr<LuaASTVisitor> visitor_;
};

/**
 * @brief 前端动作，用于创建AST消费器
 */
class LuaBindingFrontendAction : public clang::ASTFrontendAction {
public:
    explicit LuaBindingFrontendAction(std::vector<ExportInfo>* export_infos)
        : export_infos_(export_infos) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& CI, llvm::StringRef file) override {
        return std::make_unique<LuaBindingASTConsumer>(export_infos_);
    }

private:
    std::vector<ExportInfo>* export_infos_;
};

/**
 * @brief 前端动作工厂
 */
class LuaBindingActionFactory : public clang::tooling::FrontendActionFactory {
public:
    explicit LuaBindingActionFactory(std::vector<ExportInfo>* export_infos)
        : export_infos_(export_infos) {}

    std::unique_ptr<clang::FrontendAction> create() override {
        return std::make_unique<LuaBindingFrontendAction>(export_infos_);
    }

private:
    std::vector<ExportInfo>* export_infos_;
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
        if (!fs::exists(file)) {
            std::cerr << "警告: 文件不存在: " << file << std::endl;
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
        
        // 收集输入文件
        std::vector<std::string> input_files = collectInputFiles(options);
        if (input_files.empty()) {
            std::cerr << "错误: 未指定源文件" << std::endl;
            showHelp(argv[0]);
            return 1;
        }
        
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
        
        // 使用Clang工具分析源文件
        std::vector<ExportInfo> export_infos;
        
        try {
            // 创建Clang工具
            clang::tooling::ClangTool tool(compile_db, input_files);
            
            // 创建动作工厂
            auto action_factory = std::make_unique<LuaBindingActionFactory>(&export_infos);
            
            // 运行工具
            int result = tool.run(action_factory.get());
            
            if (result != 0) {
                std::cerr << "AST分析过程中出现错误" << std::endl;
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
        
        DirectBindingGenerator generator;
        DirectBindingGenerator::GenerationOptions gen_options;
        gen_options.output_directory = options.output_dir;
        // 注意：这个结构体没有module_name和verbose字段
        
        generator.SetOptions(gen_options);
        
        // 创建输出目录
        if (!fs::exists(options.output_dir)) {
            Logger::info("创建输出目录: " + options.output_dir);
            fs::create_directories(options.output_dir);
        }
        
        // 使用正确的API生成绑定
        std::string module_name = options.module_name.empty() ? "generated_module" : options.module_name;
        auto result = generator.GenerateModuleBinding(module_name, export_infos);
        
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
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "发生未知错误" << std::endl;
        return 1;
    }
}