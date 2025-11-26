//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#include "scc/ast/Codegen.hpp"
#include "scc/ast/Decl.hpp"
#include "scc/ast/Parser.hpp"
#include "scc/ast/Sema.hpp"
#include "scc/core/Logger.hpp"

#include "scc/core/Tools.hpp"
#include "spbe/X64/X64AsmWriter.hpp"
#include "spbe/analysis/TargetLoweringPass.hpp"
#include "spbe/graph/CFG.hpp"
#include "spbe/machine/AsmWriter.hpp"
#include "spbe/machine/MachObject.hpp"
#include "spbe/machine/RegisterAnalysis.hpp"
#include "spbe/target/Target.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
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
    bool link = true;
    bool assembly = false;
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
        } else if (str == "-c") {
            link = false;
        } else if (str == "-S") {
            assembly = true;
        } else {
            files.emplace_back(argv[arg]);
        }
    }

    if (files.empty())
        Logger::error("no input files");

    vector<TranslationUnitDecl*> units(files.size(), nullptr);

    Target target(Target::Arch::x64, Target::ABI::SystemV, Target::OS::Linux);

    for (uint32_t i = 0, e = files.size(); i != e; ++i) {
        const string& path = files[i];
        Parser parser(path);

        TranslationUnitDecl* unit = parser.parse();

        Sema sema(unit);
        unit->accept(sema);

        unit->print(std::cout);

        CFG graph(path, target);
        Codegen cgn(unit, graph);
        unit->accept(cgn);

        graph.print(std::cout);

        spbe::MachObject obj(&graph, &target);

        spbe::TargetLoweringPass lower(graph, obj);
        lower.run();

        spbe::RegisterAnalysis rega(obj);
        rega.run();

        string as = scc::with_assembly_extension(path);
        std::ofstream asf(as, std::ios::trunc);
        if (!asf.is_open())
            Logger::error("cannot open file: " + as);

        spbe::x64::X64AsmWriter writer(obj);
        writer.run(asf);
        asf.close();

        std::system(("/usr/bin/as -o " + scc::with_object_extension(path) + ' ' + as).c_str());

        units[i] = unit;
    }

    if (link) {
        // Use clang to link the objects together.
        //
        // TODO: Switch this out with a manual ld command and dynamically find
        // the lib paths on my own.

        string clang = "/usr/bin/clang -o " + output + ' ';

        for (const auto& file : files) {
            string filename = scc::with_object_extension(file);
            if (std::filesystem::exists(filename))
                clang += filename;
        }

        std::system(clang.c_str());

        for (const auto& file : files) {
            string filename = scc::with_object_extension(file);
            if (std::filesystem::exists(filename))
                std::filesystem::remove(filename);
        }
    }

    if (!assembly) {
        for (const auto& file : files) {
            string filename = scc::with_assembly_extension(file);
            if (std::filesystem::exists(filename))
                std::filesystem::remove(filename);
        }
    }
    
    for (TranslationUnitDecl* unit : units)
        delete unit;

    units.clear();
    files.clear();
    return 0;
}
