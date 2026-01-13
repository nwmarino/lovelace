//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/LIRCodegen.hpp"
#include "lace/codegen/LLVMCodegen.hpp"
#include "lace/core/Diagnostics.hpp"
#include "lace/core/Options.hpp"
#include "lace/parser/Parser.hpp"
#include "lace/tools/Files.hpp"
#include "lace/tree/AST.hpp"
#include "lace/tree/NameAnalysis.hpp"
#include "lace/tree/Printer.hpp"
#include "lace/tree/SemanticAnalysis.hpp"
#include "lace/tree/SymbolAnalysis.hpp"
#include "lir/analysis/LoweringPass.hpp"
#include "lir/machine/AsmWriter.hpp"
#include "lir/machine/Machine.hpp"
#include "lir/machine/RegisterAnalysis.hpp"

#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Scalar/SROA.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define LACE_VERSION_MAJOR 1
#define LACE_VERSION_MINOR 0

using namespace lace;

std::vector<AST*> compute_dependency_order(const std::vector<AST*>& asts) {
    // Map absolute file paths to their corresponding AST.
    std::unordered_map<std::string, AST*> path_to_ast = {};
    path_to_ast.reserve(asts.size());
    for (AST* ast : asts)
        path_to_ast[std::filesystem::absolute(ast->get_file()).string()] = ast;

    // Map each AST to the ASTs it loads/depends on, using resolved absolute paths.
    std::unordered_map<AST*, std::vector<AST*>> deps;
    for (AST* ast : asts) {
        std::filesystem::path parent_path = 
            std::filesystem::absolute(ast->get_file()).parent_path();
    
        for (Defn* defn : ast->get_defns()) {
            LoadDefn* load = dynamic_cast<LoadDefn*>(defn);
            if (!load)
                continue;

            std::filesystem::path loaded_path = parent_path / load->get_path();
            loaded_path = std::filesystem::weakly_canonical(loaded_path);
            auto it = path_to_ast.find(loaded_path.string());
            if (it != path_to_ast.end()) {
                deps[ast].push_back(it->second);
                load->set_path(loaded_path.string());
            } else {
                log::fatal("unresolved file: " + loaded_path.string(), 
                    log::Span(ast->get_file(), load->get_span()));
            }
        }
    }

    std::vector<AST*> order;
    std::unordered_set<AST*> visited;
    std::unordered_set<AST*> visiting;

    std::function<void(AST*)> dfs = [&](AST* ast) {
        if (visited.count(ast)) 
            return;

        if (visiting.count(ast))
            log::fatal("cyclic dependency found");
        
        visiting.insert(ast);
        
        for (AST* dep : deps[ast])
            dfs(dep);

        visiting.erase(ast);
        visited.insert(ast);
        order.push_back(ast);
    };

    for (AST* ast : asts)
        dfs(ast);

    return order;
}

std::vector<NamedDefn*> get_public_symbols(
        const std::unordered_map<std::string, AST*>& path_to_asts, AST* ast) {
    std::vector<NamedDefn*> res = {};

    for (Defn* defn : ast->get_defns()) {
        if (LoadDefn* load = dynamic_cast<LoadDefn*>(defn)) {
            AST* dep = path_to_asts.at(load->get_path());

            std::vector<NamedDefn*> dep_symbols = get_public_symbols(path_to_asts, dep);
            for (NamedDefn* sym : dep_symbols)
                res.push_back(sym);
        } else if (NamedDefn* named = dynamic_cast<NamedDefn*>(defn)) {
            if (named->has_rune(Rune::Public))
                res.push_back(named);
        }
    }

    return res;
}

/// Resolve the definition dependencies for each tree in |asts|. Assumes that
/// |asts| contains syntax trees per their dependency order.
void resolve_dependencies(const Options& options, std::vector<AST*>& asts) {
    std::unordered_map<std::string, AST*> path_to_asts;
    for (AST* ast : asts)
        path_to_asts[ast->get_file()] = ast;

    for (AST* ast : asts) {
        Scope* scope = ast->get_scope();
        std::vector<NamedDefn*> deps = {};

        // For each load in this ast, recursively collect all public 
        // definitions from the target source file.
        for (Defn* defn : ast->get_defns()) {
            LoadDefn* load = dynamic_cast<LoadDefn*>(defn);
            if (!load)
                continue;

            // Resolve the dependency based on the load path.
            AST* dep = path_to_asts[load->get_path()];
            assert(dep);

            // Fetch all public symbols and add them as dependencies.
            std::vector<NamedDefn*> syms = get_public_symbols(path_to_asts, dep);
            for (NamedDefn* sym : syms)
                deps.push_back(sym);
        }

        // For each named dependency, add it to the scope of this ast and to
        // its list of loaded symbols.
        for (NamedDefn* dep : deps) {
            if (!scope->add(dep)) {
                log::warn("'" + dep->get_name() + 
                    "' conflicts name-wise with another symbol, skipping load");
            } else {
                ast->get_loaded().push_back(dep);
                log::note("added '" + dep->get_name() + "' to file " + ast->get_file());
            }
        }

        NameAnalysis nama(options);
        nama.visit(*ast);
    }
}

void drive_lir_backend(const Options& options, const std::vector<AST*>& asts) {
    lir::Machine mach(lir::Machine::Linux);

    for (AST* ast : asts) {
        lir::CFG cfg(mach, ast->get_file());
        Codegen cgn(options, cfg);
        cgn.visit(*ast);

        if (options.print_ir) {
            std::ofstream file(ast->get_file() + ".lir");
            if (!file || !file.is_open())
                log::fatal("failed to open: " + ast->get_file() + ".s");
            
            cfg.print(file);
            file.close();
        }

        lir::Segment seg(cfg);

        lir::LoweringPass lowering(cfg, seg);
        lowering.run();

        lir::RegisterAnalysis rega(seg);
        rega.run();

        std::ofstream out(ast->get_file() + ".s");
        if (!out || !out.is_open())
            log::fatal("failed to open: " + ast->get_file() + ".s");

        lir::AsmWriter writer(seg);
        writer.run(out);
        out.close();
    }
}

void drive_llvm_backend(const Options& options, const std::vector<AST*>& asts) {
    assert(options.llvm);

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    llvm::Triple triple = llvm::Triple("amd64", "AMD", "Linux");
    llvm::TargetOptions topts;
    std::string err;

    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(
        triple.getTriple(), err);

    if (!target)
        log::fatal("failed to find target machine: " + err);

    llvm::CodeGenOptLevel opt;
    switch (options.opt) {
        case Options::OptLevel::None:
            opt = llvm::CodeGenOptLevel::None;
            break;
        case Options::OptLevel::Default:
        case Options::OptLevel::Space:
            opt = llvm::CodeGenOptLevel::Default;
            break;
        case Options::OptLevel::Few:
            opt = llvm::CodeGenOptLevel::Less;
            break;
        case Options::OptLevel::Many:
            opt = llvm::CodeGenOptLevel::Aggressive;
            break;
    }

    llvm::TargetMachine* mach = target->createTargetMachine(
        triple, 
        "generic", // cpu
        "", // features
        topts, // target options
        llvm::Reloc::PIC_, // relocation model
        std::nullopt, // code model
        opt,  // optimization level
        false // jit
    );

    std::vector<llvm::LLVMContext*> contexts = {};
    std::vector<llvm::Module*> modules = {};
    contexts.reserve(asts.size());
    modules.reserve(asts.size());

    for (AST* ast : asts) {
        llvm::LLVMContext* ctx = new llvm::LLVMContext();
        llvm::Module* mod = new llvm::Module(ast->get_file(), *ctx);
        
        LLVMCodegen CGN(options, ast, mod);
        CGN.finalize();

        mod->setDataLayout(mach->createDataLayout());
        mod->setTargetTriple(triple);

        if (options.print_ir) {
            std::error_code err;
            llvm::raw_fd_ostream ir(ast->get_file() + ".ll", err);
            if (err)
                log::fatal("failed to open file: " + ast->get_file() + ".ll");
            
            mod->print(ir, nullptr);
            ir.close();
        }

        contexts.push_back(ctx);
        modules.push_back(mod);
    }

    for (llvm::Module* mod : modules) {
        llvm::PassBuilder builder(mach);

        llvm::LoopAnalysisManager LAM;
        llvm::FunctionAnalysisManager FAM;
        llvm::CGSCCAnalysisManager CGAM;
        llvm::ModuleAnalysisManager MAM;

        llvm::TargetLibraryInfoImpl TLII(triple);
        TLII.disableAllFunctions();

        FAM.registerPass([&] {
            return llvm::TargetLibraryAnalysis(TLII);
        });

        FAM.registerPass([&] {
            return builder.buildDefaultAAPipeline();
        });

        builder.registerModuleAnalyses(MAM);
        builder.registerCGSCCAnalyses(CGAM);
        builder.registerFunctionAnalyses(FAM);
        builder.registerLoopAnalyses(LAM);
        builder.crossRegisterProxies(LAM, FAM, CGAM, MAM);

        llvm::ModulePassManager MPM;
        MPM.addPass(llvm::createModuleToFunctionPassAdaptor(
            llvm::SROAPass(llvm::SROAOptions::ModifyCFG)));
        MPM.addPass(llvm::createModuleToFunctionPassAdaptor(
            llvm::SimplifyCFGPass()));

        std::string out = mod->getSourceFileName() + ".s";
        llvm::sys::fs::OpenFlags flags = llvm::sys::fs::OF_Text;

        std::error_code err;
        llvm::ToolOutputFile* output = new llvm::ToolOutputFile(mod->getSourceFileName() + ".s", err, flags);
        assert(!err);

        llvm::legacy::PassManager LPM;
        if (mach->addPassesToEmitFile(LPM, output->os(), nullptr, llvm::CodeGenFileType::AssemblyFile))
            log::fatal("failed to compile: " + mod->getSourceFileName());

        MPM.run(*mod, MAM);
        LPM.run(*mod);
        output->keep();

        if (options.verbose)
            log::note("finished compilation for: " + mod->getSourceFileName());
    }

    for (llvm::Module* mod : modules)
        delete mod;

    for (llvm::LLVMContext* ctx : contexts)
        delete ctx;
}

int32_t main(int32_t argc, char** argv) {
    Options options;
    options.output = "main";
    options.opt = Options::OptLevel::None;
    options.debug = true;
    options.time = true;
    options.verbose = true;
    options.version = true;
    options.llvm = false;
    options.print_tree = false;
    options.print_ir = true;

    log::init();

    if (options.version)
        log::note("version: " + std::to_string(LACE_VERSION_MAJOR) + '.' + 
            std::to_string(LACE_VERSION_MINOR));

    std::vector<std::string> files = { 
        "/home/statim/lace/samples/A.lace", 
        "/home/statim/lace/samples/linux.lace",
        "/home/statim/lace/samples/mem.lace",
    };

    for (int32_t i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-o") {
            if (i + 1 == argc)
                log::fatal("expected filename after -o");

            options.output = argv[++i];
        } else {
            if (arg.size() < 4 || arg.substr(arg.size() - 5) != ".lace")
                log::error("expected source file ending with \".lace\", got " + arg);

            files.push_back(std::filesystem::absolute(arg).string());
        }
    }

    if (files.empty())
        log::fatal("no input files");

    log::flush();

    std::vector<AST*> asts = {};
    asts.reserve(files.size());

    for (const std::string& file : files) {
        Parser parser(read_file(file), file);
        asts.push_back(parser.parse());
        
        if (options.verbose)
            log::note("parsed file: " + file);
    }

    log::flush();

    std::vector<AST*> ordered = compute_dependency_order(asts);
    resolve_dependencies(options, ordered);
    
    log::flush();

    for (AST* ast : ordered) {
        SymbolAnalysis syma(options);
        ast->accept(syma);

        if (options.verbose)
            log::note("ran symbol analysis on file: " + ast->get_file());
    }

    log::flush();

    for (AST* ast : ordered) {
        SemanticAnalysis sema(options);
        ast->accept(sema);

        if (options.verbose)
            log::note("ran semantic analysis on file: " + ast->get_file());

        if (options.print_tree) {
            Printer printer(options, std::cout);
            printer.visit(*ast);
        }
    }

    log::flush();

    if (options.llvm) {
        drive_llvm_backend(options, asts);
    } else {
        // Default to LIR.
        drive_lir_backend(options, asts);
    }

    for (AST* ast : asts)
        delete ast;

    asts.clear();
    files.clear();

    return 0;
}
