//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.hpp"
#include "lace/core/Options.hpp"
#include "lace/parser/Parser.hpp"
#include "lace/tools/Files.hpp"
#include "lace/tree/AST.hpp"
#include "lace/tree/SemanticAnalysis.hpp"
#include "lace/tree/SymbolAnalysis.hpp"

#include <cstdint>
#include <string>
#include <vector>

#define LACE_VERSION_MAJOR 1
#define LACE_VERSION_MINOR 0

using namespace lace;

int32_t main(int32_t argc, char** argv) {
    Options options;
    options.output = "main";
    options.opt = Options::OptLevel::None;
    options.debug = true;
    options.time = true;
    options.verbose = true;
    options.version = true;
    options.print_tree = false;
    options.print_spbe = false;

    log::init();

    if (options.version)
        log::note("version: " + std::to_string(LACE_VERSION_MAJOR) + '.' + 
            std::to_string(LACE_VERSION_MINOR));

    std::vector<std::string> files = { "samples/return_zero.stm" };
    std::vector<AST*> asts = {};
    asts.reserve(files.size());

    for (int32_t i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-o") {
            if (i + 1 == argc)
                log::fatal("expected filename after -o");

            options.output = argv[++i];
        } else {
            files.push_back(arg);
        }
    }

    if (files.empty())
        log::fatal("no input files");

    for (const std::string& file : files) {
        Parser parser(read_file(file), file);
        asts.push_back(parser.parse());
        
        if (options.verbose)
            log::note("parsed file: " + file);
    }
    
    for (AST* ast : asts) {
        SymbolAnalysis syma(options);
        ast->accept(syma);

        if (options.verbose)
            log::note("ran symbol analysis on file: " + ast->get_file());
    }

    for (AST* ast : asts) {
        SemanticAnalysis sema(options);
        ast->accept(sema);

        if (options.verbose)
            log::note("ran semantic analysis on file: " + ast->get_file());
    }

    for (AST* ast : asts)
        delete ast;

    asts.clear();
    files.clear();

    return 0;
}
