//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/codegen/SPBECodegen.hpp"
#include "stmc/core/Diagnostics.hpp"
#include "stmc/core/Options.hpp"
#include "stmc/parser/Parser.hpp"
#include "stmc/tools/Files.hpp"
#include "stmc/tree/Context.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/SemanticAnalysis.hpp"
#include "stmc/tree/SymbolAnalysis.hpp"

#include "spbe/graph/CFG.hpp"
#include "spbe/target/Target.hpp"
#include "spbe/X64/X64AsmWriter.hpp"
#include "spbe/analysis/TargetLoweringPass.hpp"
#include "spbe/machine/MachObject.hpp"
#include "spbe/machine/RegisterAnalysis.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#define STMC_VERSION_MAJOR 1
#define STMC_VERSION_MINOR 0

using namespace stm;
using namespace spbe;

using std::ofstream;
using std::string;
using std::vector;

int32_t main(int32_t argc, char** argv) {
    Diagnostics diags;

    Options options;
    options.output = "main";
    options.opt_level = Options::OptLevel::None;
    options.stop = Options::StopPoint::None;
    options.debug = true;
    options.time = true;
    options.verbose = true;
    options.version = true;

    options.print_tree = false;
    options.print_spbe = false;

    if (options.version)
        diags.log("version: " + std::to_string(STMC_VERSION_MAJOR) + '.' + 
            std::to_string(STMC_VERSION_MINOR));

    vector<string> files = { "samples/return_zero.stm" };
    vector<TranslationUnitDecl*> units = {};

    for (int32_t i = 1; i < argc; ++i)
        files.push_back(string(argv[i]));
    
    if (files.empty())
        diags.fatal("no input files");

    for (const string& file : files) {
        Parser parser(diags, file);
        units.push_back(parser.parse());
        
        if (options.verbose)
            diags.note("parsed file: " + file);
    }
    
    for (TranslationUnitDecl* unit : units) {
        SymbolAnalysis syma(diags, options);
        unit->accept(syma);

        if (options.verbose)
            diags.note("ran symbol analysis on file: " + unit->get_file());
    }

    for (TranslationUnitDecl* unit : units) {
        SemanticAnalysis sema(diags, options);
        unit->accept(sema);

        if (options.verbose)
            diags.note("ran semantic analysis on file: " + unit->get_file());
    }

    Target target(Target::Arch::x64, Target::ABI::SystemV, Target::OS::Linux);

    for (uint32_t i = 0, e = files.size(); i < e; ++i) {
        CFG graph(files[i], target);

        SPBECodegen cgn(diags, options, graph);
        units[i]->accept(cgn);

        if (options.verbose)
            diags.note("generated code for " + files[i]);

        graph.print(std::cout);

        spbe::MachObject obj(&graph, &target);

        spbe::TargetLoweringPass lower(graph, obj);
        lower.run();

        spbe::RegisterAnalysis rega(obj);
        rega.run();

        ofstream out(with_assembly_extension(files[i]));
        if (!out.is_open())
            diags.fatal("unable to create file: " + with_assembly_extension(files[i]));

        spbe::x64::X64AsmWriter writer(obj);
        writer.run(out);

        out.close();
    }

    if (options.stop == Options::StopPoint::Assembly)
        goto CLEANUP;

    for (uint32_t i = 0, e = files.size(); i < e; ++i) {
        const string& file = files[i];
        std::system(string("as -o " + with_object_extension(file) + ' ' + 
            with_assembly_extension(file)).c_str());
    }

    for (uint32_t i = 0, e = files.size(); i < e; ++i)
        std::filesystem::remove(with_assembly_extension(files[i]));

    if (options.stop == Options::StopPoint::Object)
        goto CLEANUP;

    {
        string link = "ld -o " + options.output + " std/rt.o";
        for (uint32_t i = 0, e = files.size(); i < e; ++i)
            link += ' ' + with_object_extension(files[i]);
        
        std::system(link.c_str());

        for (uint32_t i = 0, e = files.size(); i < e; ++i)
            std::filesystem::remove(with_object_extension(files[i]));
    }

CLEANUP:
    for (TranslationUnitDecl* unit : units)
        delete unit;

    units.clear();
    files.clear();

    return 0;
}
