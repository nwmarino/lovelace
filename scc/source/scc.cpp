//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#include "ast/Parser.hpp"
#include "core/Logger.hpp"
#include "core/TranslationUnit.hpp"

#include <cstdint>
#include <string>
#include <vector>

using scc::Logger;
using scc::Parser;
using scc::TranslationUnit;

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

    std::vector<std::unique_ptr<TranslationUnit>> units = {};
    units.reserve(files.size());

    for (const auto& file : files) {
        Parser parser(file);

        auto unit = std::unique_ptr<TranslationUnit>(new TranslationUnit());
        parser.parse(*unit);

        unit->print(std::cout);

        units.push_back(std::move(unit));
    }

    return 0;
}
