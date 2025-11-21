//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#include "core/Logger.hpp"

#include <cstdint>
#include <string>
#include <vector>

using scc::Logger;

int32_t main(int32_t argc, char* argv[]) {
    Logger::init();

    std::string output = "main";
    std::vector<std::string> files = {};

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

    return 0;
}
