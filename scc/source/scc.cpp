//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#include "scc/ast/Decl.hpp"
#include "scc/ast/Parser.hpp"
#include "scc/ast/Sema.hpp"
#include "scc/core/Logger.hpp"

#include <cstdint>
#include <string>
#include <vector>

using std::vector;

using scc::Logger;
using scc::Parser;
using scc::Sema;
using scc::TranslationUnitDecl;

int32_t main(int32_t argc, char* argv[]) {
    Logger::init();

    std::string output = "main";
    std::vector<std::string> files = {};

#ifdef SCC_COMPILE_SAMPLES
    files.push_back("samples/binary.c");
    files.push_back("samples/control_flow.c");
    files.push_back("samples/literals.c");
    files.push_back("samples/return_zero.c");
    files.push_back("samples/unary.c");
    files.push_back("samples/redef.c");
#endif // SCC_COMPILE_SAMPLES

    for (int32_t arg = 1; arg < argc; ++arg) {
        std::string str = std::string(argv[arg]);

        if (str == "-o") {
            if (++arg == argc)
                Logger::error("expected output name after '-o'");

            output = std::string(argv[arg]);
        } else {
            files.emplace_back(argv[arg]);
        }
    }

    if (files.empty())
        Logger::error("no input files");

    vector<TranslationUnitDecl*> units(files.size(), nullptr);

    for (uint32_t i = 0, e = files.size(); i != e; ++i) {
        Parser parser(files[i]);

        TranslationUnitDecl* unit = parser.parse();

        Sema sema(unit);
        unit->accept(sema);

        unit->print(std::cout);

        units[i] = unit;
    }

    return 0;
}
