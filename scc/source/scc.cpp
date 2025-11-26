//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#include "scc/ast/Codegen.hpp"
#include "scc/ast/Decl.hpp"
#include "scc/ast/Parser.hpp"
#include "scc/ast/Sema.hpp"
#include "scc/core/Logger.hpp"

#include "spbe/graph/CFG.hpp"
#include "spbe/target/Target.hpp"

#include <cstdint>
#include <string>
#include <vector>

using std::string;
using std::vector;

using scc::Codegen;
using scc::Logger;
using scc::Parser;
using scc::Sema;
using scc::TranslationUnitDecl;

using spbe::CFG;
using spbe::Target;

int32_t main(int32_t argc, char* argv[]) {
    Logger::init();

    string output = "main";
    vector<string> files = {};

#ifdef SCC_COMPILE_SAMPLES
    files.push_back("/home/nwm/statim/samples/return_zero.c");
#endif // SCC_COMPILE_SAMPLES

    for (int32_t arg = 1; arg < argc; ++arg) {
        string str = argv[arg];

        if (str == "-o") {
            if (++arg == argc)
                Logger::error("expected output name after '-o'");

            output = argv[arg];
        } else {
            files.emplace_back(argv[arg]);
        }
    }

    if (files.empty())
        Logger::error("no input files");

    vector<TranslationUnitDecl*> units(files.size(), nullptr);

    Target target(Target::Arch::x64, Target::ABI::SystemV, Target::OS::Linux);

    for (uint32_t i = 0, e = files.size(); i != e; ++i) {
        Parser parser(files[i]);

        TranslationUnitDecl* unit = parser.parse();

        Sema sema(unit);
        unit->accept(sema);

        unit->print(std::cout);

        CFG graph(files[i], target);
        Codegen cgn(unit, graph);
        unit->accept(cgn);

        graph.print(std::cout);

        units[i] = unit;
    }

    for (TranslationUnitDecl* unit : units)
        delete unit;

    units.clear();
    files.clear();
    return 0;
}
