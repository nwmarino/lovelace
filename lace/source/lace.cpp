//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/LIRCodegen.hpp"
#include "lace/core/Diagnostics.hpp"
#include "lace/core/ThreadPool.hpp"
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

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define LACE_VERSION_MAJOR 1
#define LACE_VERSION_MINOR 0

using namespace lace;
using namespace std::filesystem;

using Asts = std::unordered_set<AST*>;
using DepTable = std::unordered_map<AST*, Asts>;
using FileTable = std::unordered_map<std::string, AST*>;

struct InputFile final {
    std::string file;
    AST* ast;

    InputFile(const std::string& file, AST* ast = nullptr) 
      : file(file), ast(ast) {}
};

/// A mapping between the absolute path of an input file and its parsed AST.
static FileTable g_files = {};

/// Setup |g_files| based on the set of given |asts| and their respective
/// input files.
void setup_file_table(const Asts& asts) {
    g_files.reserve(asts.size());
    for (AST* ast : asts) {
        g_files.emplace(
            std::filesystem::absolute(ast->get_file()).string(), ast); 
    }
}

/// Compute the dependency order and a dependency table for each file in the 
/// list of |asts|.
///
/// This function produces an ordering of the files without cycles to 
/// |ordering|, s.t. any given file relies on only the ones which come 
/// before it in the set. This means the result is a valid ordering in which to 
/// sequentially perform name analysis on each syntax tree.
///
/// Moreover, as it computes dependencies, it saves them to |deps|.
void compute_dependencies(const Asts& asts, Asts& ordering, DepTable& deps) {
    for (AST* ast : asts) {
        path parent = absolute(ast->get_file()).parent_path();
    
        for (Defn* defn : ast->get_defns()) {
            LoadDefn* load = dynamic_cast<LoadDefn*>(defn);
            if (!load)
                continue;

            // Find the canonical path for the target file.
            path target = parent / load->get_path();
            target = weakly_canonical(target);

            auto it = g_files.find(target.string());
            if (it != g_files.end()) {
                deps[ast].insert(it->second);
                load->set_path(target.string());
            } else {
                log::fatal("unresolved file: " + target.string(), 
                    log::Span(ast->get_file(), load->get_span()));
            }
        }
    }

    Asts visited = {};
    Asts visiting = {};

    std::function<void(AST*)> dfs = [&](AST* ast) {
        if (visited.count(ast)) 
            return;

        if (visiting.count(ast))
            log::fatal("cyclic dependency found", 
                log::Location(ast->get_file(), { 1, 1 }));
        
        visiting.insert(ast);
        
        for (AST* dep : deps[ast])
            dfs(dep);

        visiting.erase(ast);
        visited.insert(ast);
        ordering.insert(ast);
    };

    for (AST* ast : asts)
        dfs(ast);
}

/// Resolve the dependent symbols for each tree in |asts|, based on their
/// dependencies defined in |deps|. Assumes that |asts| contains syntax 
/// trees in their dependency order.
void resolve_dependencies(const Options& options, const Asts& asts, 
                          const DepTable& deps) {
    for (AST* ast : asts) {
        Asts dep_list = deps.at(ast);
        std::vector<NamedDefn*> symbols = {};

        // For each dependency, fetch all of its public, named definitions.
        for (AST* dep : dep_list) {
            for (Defn* defn : dep->get_defns()) {
                NamedDefn* symbol = dynamic_cast<NamedDefn*>(defn);
                if (symbol && symbol->has_rune(Rune::Public))
                    symbols.push_back(symbol);
            }
        }

        Scope* scope = ast->get_scope();
        for (NamedDefn* symbol : symbols) {
            bool res = scope->add(symbol);
            if (!res) {
                log::fatal("name-wise conflict with an existing definition: " 
                    + symbol->get_name(), log::Location(ast->get_file(), { 1, 1 }));
            }

            ast->get_loaded().push_back(symbol);
            
            if (options.verbose) {
                log::note("added '" + symbol->get_name() + "' to " + ast->get_file());
            }
        }

        if (options.verbose)
            log::note("running name analysis on: " + ast->get_file());

        NameAnalysis name_analysis(options);
        ast->accept(name_analysis);

        if (options.verbose)
            log::note("finished name analysis for: " + ast->get_file());
    }
}

void drive_lir_backend(const Options& options, const Asts& asts) {
    lir::Machine mach(lir::Machine::Linux);

    for (AST* ast : asts) {
        lir::CFG cfg(mach, ast->get_file());

        if (options.verbose)
            log::note("running code generation for: " + ast->get_file());

        LIRCodegen codegen(options, ast, cfg);
        codegen.run();

        if (options.verbose)
            log::note("finished code generation for: " + ast->get_file());

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

        std::ofstream as(ast->get_file() + ".s");
        if (!as || !as.is_open())
            log::fatal("failed to open: " + ast->get_file() + ".s");

        lir::AsmWriter writer(seg);
        writer.run(as);
        as.close();

        std::string assembler = "as " + ast->get_file() + ".s -o " + ast->get_file() + ".o";
        std::system(assembler.c_str());
    }
}

int32_t main(int32_t argc, char** argv) {
    Options options;
    options.output = "main";
    options.opt = Options::OptLevel::None;
    options.threads = 1;

    options.debug = true;
    options.multithread = true;
    options.time = true;
    options.verbose = true;
    options.version = true;
    options.llvm = false;
    options.print_tree = true;
    options.print_ir = true;

    log::init();

    std::vector<InputFile> files = {};

    for (int32_t i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-b") {
            options.verbose = true;
        } else if (arg == "-g") {
            options.debug = true;
        } else if (arg == "-t") {
            options.time = true;
        } else if (arg == "-v") {
            log::note("version: " + std::to_string(LACE_VERSION_MAJOR) + "." + 
                std::to_string(LACE_VERSION_MINOR));
        } else if (arg == "-O0") {
            options.opt = Options::OptLevel::None;
        } else if (arg == "-O1") {
            options.opt = Options::OptLevel::Few;
        } else if (arg == "-O2") {
            options.opt = Options::OptLevel::Default;
        } else if (arg == "-O3") {
            options.opt = Options::OptLevel::Many;
        } else if (arg == "-Os") {
            options.opt = Options::OptLevel::Space;
        } else if (arg == "-st") {
            options.multithread = false;
        } else if (arg == "-dump-ast") {
            options.print_tree = true;
        } else if (arg == "-dump-ir") {
            options.print_ir = true;
        } else if (arg == "-j") {
            if (i + 1 == argc)
                log::fatal("expected number after -j");

            int32_t threads = std::stoi(argv[++i]);
            if (threads <= 0)
                log::fatal("thread count must be a positive number, got " 
                    + std::to_string(threads));

            options.threads = static_cast<uint32_t>(threads);
        } else if (arg == "-o") {
            if (i + 1 == argc)
                log::fatal("expected filename after -o");

            options.output = argv[++i];
        } else {
            if (arg.size() < 4 || arg.substr(arg.size() - 5) != ".lace")
                log::error("expected source file ending with \".lace\", got " + arg);

            files.push_back(absolute(arg).string());
        }
    }

    if (files.empty())
        log::fatal("no input files");

    log::flush();

    if (options.multithread) {
        const uint32_t supported_threads = std::thread::hardware_concurrency(); 

        if (options.threads == 1) {
            // If no -j was provided, then take the larger of 1 and the 
            // detected thread count.
            options.threads = std::max(1u, supported_threads);
        } else {
            // A -j was provided, but if it's larger than the number of threads 
            // supported, then use only what's available.
            options.threads = std::min(options.threads, supported_threads);
        }

        // No point in us using more threads than there are files.
        options.threads = std::min(options.threads, 
            static_cast<uint32_t>(files.size()));
    }

    ThreadPool* pool = nullptr;
    if (options.multithread) {
        pool = new ThreadPool(options.threads);
        assert(pool);

        if (options.verbose)
            log::note("using " + std::to_string(options.threads) + " threads");
    }

    if (options.multithread) {
        assert(pool);

        for (InputFile& f : files) {
            pool->push([&f, options] {
                if (options.verbose)
                    log::note("parsing file: " + f.file);

                Parser parser(read_file(f.file), f.file);
                f.ast = parser.parse();

                if (options.verbose)
                    log::note("finishing parsing for: " + f.file);
            });
        }

        pool->wait();
    } else for (InputFile& f : files) {
        if (options.verbose)
            log::note("parsing file: " + f.file);

        Parser parser(read_file(f.file), f.file);
        f.ast = parser.parse();
        
        if (options.verbose)
            log::note("finishing parsing for: " + f.file);
    }

    log::flush();

    Asts asts = {};
    asts.reserve(files.size());
    for (InputFile& file : files)
        asts.insert(file.ast);

    setup_file_table(asts);

    Asts ordering = {};
    DepTable deps = {};
    ordering.reserve(asts.size());
    deps.reserve(asts.size());

    compute_dependencies(asts, ordering, deps);
    resolve_dependencies(options, ordering, deps);

    // Perform symbol analysis on each syntax tree.
    //
    // @Todo: expirement if this needs the dependency ordering or not.
    for (AST* ast : asts) {
        if (options.verbose)
            log::note("running symbol analysis on: " + ast->get_file());

        SymbolAnalysis symbol_analysis(options);
        ast->accept(symbol_analysis);

        if (options.verbose)
            log::note("finished symbol analysis for: " + ast->get_file());
    }

    log::flush();

    // Perform semantic analysis on each syntax tree.
    for (AST* ast : asts) {
        if (options.verbose)
            log::note("running semantic analysis on: " + ast->get_file());

        SemanticAnalysis semantic_analysis(options);
        ast->accept(semantic_analysis);

        if (options.verbose)
            log::note("finished semantic analysis for: " + ast->get_file());

        // AST is now considered valid, so print it if needbe.
        if (options.print_tree) {
            std::ofstream out(ast->get_file() + ".ast");
            if (!out || !out.is_open())
                log::fatal("failed to open file: " + ast->get_file() + ".ast");

            Printer printer(options, out);
            ast->accept(printer);
            out.close();
        }
    }

    log::flush();

    if (options.llvm) {
        //drive_llvm_backend(options, asts);
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
