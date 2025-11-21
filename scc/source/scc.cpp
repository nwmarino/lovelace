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

    std::vector<std::string> files = {};

    for (int32_t arg = 1; arg < argc; ++arg) {
        files.emplace_back(argv[arg]);
    }

    if (files.empty())
        Logger::error("no input files.");

    return 0;
}
